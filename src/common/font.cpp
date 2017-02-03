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
// font.cpp
////////////////////////////////////////////////////////////////////////

#include "font.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Font::~Font
// DESCRIPTION: Destroy font.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Font::~Font( void ) {
  if ( f ) TTF_CloseFont( f );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::Load
// DESCRIPTION: Load a font from a data file.
// PARAMETERS : file - name of the font file
//              size - requested font size
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Font::Load( const char *file, int size ) {
  // SDL_ttf segfaults if the font is not present...
  if ( File::Exists( file ) ) {

    f = TTF_OpenFont( file, size );

    if ( f ) {
      col = Color(CF_COLOR_WHITE);

      TTF_SetFontStyle( f, TTF_STYLE_BOLD );

      // this is an ugly hack and rather error prone...
      int minx, maxx;
      TTF_GlyphMetrics( f, 'w', &minx, &maxx, 0, 0, 0 );
      width = maxx - minx;

      height = TTF_FontHeight( f );
      return 0;
    }
  }
  return -1;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::CharWidth
// DESCRIPTION: Get the pixel width of a single character.
// PARAMETERS : c - character
// RETURNS    : character width
////////////////////////////////////////////////////////////////////////

unsigned char Font::CharWidth( char c ) const {
  char str[2] = { c, 0 };
  return TextWidth( str );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::TextWidth
// DESCRIPTION: Calculate the length of a string in pixels.
// PARAMETERS : str - string to get the length of
// RETURNS    : length of str in pixels
////////////////////////////////////////////////////////////////////////

unsigned short Font::TextWidth( const char *str ) const {
  if ( !str ) return 0;

  unsigned short maxw = 0;
  string buf( str );
  size_t pos, prev = 0;

  do {
    pos = buf.find( '\n', prev );
    int w;

    string sub( buf.substr( prev, pos - prev ) );

    TTF_SizeUTF8( f, sub.c_str(), &w, 0 );

    if ( w > maxw ) maxw = w;
    prev = pos + 1;
  } while ( pos != string::npos );
  return maxw;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::TextHeight
// DESCRIPTION: Calculate the height of a string in pixels.
// PARAMETERS : str     - string to get the height of; A new line is
//                        started whenever the length exceeds the
//                        available line width or a newline character
//                        is encountered
//              linew   - available line width in pixels
//              spacing - vertical spacing between lines
// RETURNS    : height of str in pixels
////////////////////////////////////////////////////////////////////////

unsigned short Font::TextHeight( const char *str, unsigned short linew,
                                 unsigned short spacing ) const {
  unsigned short lines = 0;
  int pos = 0, endpos = strlen(str);

  while ( pos < endpos ) {
    pos += FitText( &str[pos], linew, true );
    ++lines;
  }

  return lines * (Height() + spacing);
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::FitText
// DESCRIPTION: Check how much of a string fits into a display.
// PARAMETERS : str   - string to check
//              width - display width
//              word  - if TRUE, break string at SPACE and NEWLINE only,
//                      otherwise break whenever linelength exceeds
//                      display width
// RETURNS    : number of characters that fit into the display
////////////////////////////////////////////////////////////////////////

unsigned short Font::FitText( const char *str, unsigned short width,
                              bool word ) const {
  int lastspace = 0, pos = 0;
  string line;

  do {
    if ( (str[pos] == ' ') || (str[pos] == '\n') || (str[pos] == '\0') ) {
      if ( TextWidth( line.c_str() ) > width ) {
        // go back to last space
        line.erase( lastspace );
        pos = lastspace;

        if ( lastspace != 0 && word ) return pos + 1;
        else {
          // add single characters until we hit the wall
          do {
            line += str[pos++];
          } while ( TextWidth( line.c_str() ) <= width );
          return pos - 1;
        }
      }

      if ( (str[pos] == '\n') || (str[pos] == '\0') ) return pos + 1;
      lastspace = pos;
      line += str[pos];
    }

    else line += str[pos];

  } while ( str[pos++] != '\0' );

  return pos;
}


////////////////////////////////////////////////////////////////////////
// NAME       : Font::SetColor
// DESCRIPTION: Set the foreground color of the font.
// PARAMETERS : fcol - new font color
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Font::SetColor( const Color &fcol ) {
  col = fcol;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::Write
// DESCRIPTION: Print a string to a surface.
// PARAMETERS : str  - string
//              dest - destination surface
//              x    - left edge of string
//              y    - top edge of string
// RETURNS    : number of horizontal pixels written
////////////////////////////////////////////////////////////////////////

int Font::Write( const char *str, Surface *dest, short x, short y ) const {
  SDL_Color scol = { col.r, col.g, col.b };
  SDL_Surface *s = TTF_RenderUTF8_Blended( f, str, scol );
  if ( s ) {
    SDL_Rect src = { 0, 0, s->w, s->h };
    SDL_Rect dst = { x, y, s->w, s->h };
    SDL_BlitSurface( s, &src, dest->s_surface, &dst );
    SDL_FreeSurface( s );
    return src.w;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::Write
// DESCRIPTION: Print a string to a surface with clipping.
// PARAMETERS : str  - string
//              dest - destination surface
//              x    - left edge of string
//              y    - top edge of string
//              clip - clipping rectangle
// RETURNS    : number of horizontal pixels written
////////////////////////////////////////////////////////////////////////

int Font::Write( const char *str, Surface *dest, short x, short y,
                  const Rect &clip ) const {
  if ( (y + height < clip.y) || (y >= clip.y + clip.h) ||
       (x >= clip.x + clip.w) ) return 0;

  Rect oldclip;
  dest->GetClipRect( oldclip );
  dest->SetClipRect( clip );
  int px = Write( str, dest, x, y );
  dest->SetClipRect( oldclip );
  return px;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::Write
// DESCRIPTION: Print a colored string to a surface.
// PARAMETERS : str   - string
//              dest  - destination surface
//              x     - left edge of string
//              y     - top edge of string
//              color - string color
// RETURNS    : number of horizontal pixels written
////////////////////////////////////////////////////////////////////////

int Font::Write( const char *str, Surface *dest, short x, short y,
                  const Color &color ) {
  Color old = col;
  SetColor( color );
  int px = Write( str, dest, x, y );
  SetColor( old );
  return px;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::Write
// DESCRIPTION: Print a character to a surface.
// PARAMETERS : c    - character
//              dest - destination surface
//              x    - left edge of destination area
//              y    - top edge of destination area
// RETURNS    : number of horizontal pixels written
////////////////////////////////////////////////////////////////////////

int Font::Write( char c, Surface *dest, short x, short y ) const {
  char str[2] = { c, 0 };
  return Write( str, dest, x, y );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::Write
// DESCRIPTION: Print a colored character to a surface.
// PARAMETERS : c     - character
//              dest  - destination surface
//              x     - left edge of destination area
//              y     - top edge of destination area
//              color - character color
// RETURNS    : number of horizontal pixels written
////////////////////////////////////////////////////////////////////////

int Font::Write( char c, Surface *dest, short x, short y,
                  const Color &color ) {
  Color old = col;
  SetColor( color );
  int px = Write( c, dest, x, y );
  SetColor( old );
  return px;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Font::WriteEllipsis
// DESCRIPTION: Print a string to a surface with clipping and ellipsize
//              (...) it if it doesn't fit.
// PARAMETERS : str  - string
//              dest - destination surface
//              x    - left edge of string
//              y    - top edge of string
//              clip - clipping rectangle
// RETURNS    : number of horizontal pixels written
////////////////////////////////////////////////////////////////////////

int Font::WriteEllipsis( const char *str, Surface *dest, short x, short y,
                  const Rect &clip ) const {
  if ( (y + height < clip.y) || (y >= clip.y + clip.h) ||
       (x >= clip.x + clip.w) ) return 0;

  if ( TextWidth( str ) + x - clip.x > clip.w ) {
    const char *dots = "...";
    int w, addw = TextWidth( dots ) + x - clip.x;

    string text( str );

    do {
      text.erase( text.length() - 1 );
      w = TextWidth( text.c_str() ) + addw;
    } while ( (w > clip.w) && (text.length() > 1) );

    text.append( dots );
    str = text.c_str();
  }
  return Write( str, dest, x, y, clip );
}

