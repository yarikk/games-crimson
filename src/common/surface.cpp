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

////////////////////////////////////////////////////////////////////////////
// surface.cpp
////////////////////////////////////////////////////////////////////////////

#include "surface.h"
#include "globals.h"

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::~Surface
// DESCRIPTION: Free the memory used by this surface.
// PARAMETERS : -
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Surface::~Surface( void ) {
  if ( s_surface ) SDL_FreeSurface( s_surface );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::Create
// DESCRIPTION: Allocate a buffer for this surface.
// PARAMETERS : w     - surface width
//              h     - surface height
//              bpp   - bits per pixel
//              flags - surface flags (see SDL_video.h for details)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::Create( short w, short h, int bpp, unsigned long flags ) {
  if ( s_surface ) SDL_FreeSurface( s_surface );
  s_surface = SDL_CreateRGBSurface( flags, w, h, bpp, 0, 0, 0, 0 );
  if ( s_surface == 0 ) return -1;
  this->w = s_surface->w;
  this->h = s_surface->h;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::SetColorKey
// DESCRIPTION: Set transparent color for this surface.
// PARAMETERS : col - transparent color
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::SetColorKey( const Color &col ) {
  if ( s_surface == 0 ) return -1;
  return SDL_SetColorKey( s_surface, SDL_SRCCOLORKEY, MapRGB( col ) );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::GetColorKey
// DESCRIPTION: Get transparent color for this surface.
// PARAMETERS : -
// RETURNS    : transparent color
////////////////////////////////////////////////////////////////////////

Color Surface::GetColorKey( void ) const {
  Uint8 r, g, b;
  SDL_GetRGB( s_surface->format->colorkey, s_surface->format, &r, &g, &b );
  return Color( r, g, b );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::GetPixel
// DESCRIPTION: Read the pixel color at a specified position.
// PARAMETERS : x - horiontal pixel position
//              y - vertical pixel position
// RETURNS    : pixel color
////////////////////////////////////////////////////////////////////////

Color Surface::GetPixel( unsigned short x, unsigned short y ) const {
  Uint8 r, g, b, bpp = s_surface->format->BytesPerPixel;
  Uint32 col = 0;

  {
    SurfaceLock lck( this );
    Uint8 *pixel = ((Uint8 *)s_surface->pixels) + y * s_surface->pitch + x * bpp;

    switch ( bpp ) {
    case 1: 
      col = *pixel;
      break;
    case 2:
      col = *((Uint16 *)pixel);
      break;
    case 3:
      if ( SDL_BYTEORDER == SDL_LIL_ENDIAN )
        col = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
      else
        col = (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
      break;
    case 4:
      col = *((Uint32 *)pixel);
      break;
    }
  }

  SDL_GetRGB( col, s_surface->format, &r, &g, &b );
  return Color( r, g, b );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::DisplayFormat
// DESCRIPTION: Convert surface format to display format.
// PARAMETERS : -
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::DisplayFormat( void ) {
  SDL_Surface *new_s;

  if ( s_surface == 0 || !SDL_WasInit( SDL_INIT_VIDEO ) )  return -1;
  if ( (new_s = SDL_DisplayFormat( s_surface )) == 0 ) return -1;
  SDL_FreeSurface( s_surface );
  s_surface = new_s;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::Blit
// DESCRIPTION: Blit parts of the surface onto another surface.
// PARAMETERS : dest - destination surface
//              from - source rectangle
//              dx   - left edge of destination rectangle
//              dy   - top edge of destination rectangle
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::Blit( Surface *dest, const Rect &from, short dx, short dy ) const {
  if ( (s_surface == 0) || (from.w == 0) || (from.h == 0) ) return -1;

  SDL_Rect src, dst;

  src.x = from.x;
  src.y = from.y;
  src.w = from.w;
  src.h = from.h;

  dst.x = dx;
  dst.y = dy;
  dst.w = from.w;
  dst.h = from.h;

  return SDL_BlitSurface( s_surface, &src, dest->s_surface, &dst );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::LowerBlit
// DESCRIPTION: This function works exactly like Surface::Blit() but it
//              calls SDL_LowerBlit() instead of SDL_BitSurface().
//              SDL_LowerBlit does not clip the blitted area to the
//              surface, so it's faster, but you must make sure the
//              blitting area does not extend beyond the surface.
// PARAMETERS : dest - destination surface
//              from - source rectangle
//              dx   - left edge of destination rectangle
//              dy   - top edge of destination rectangle
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::LowerBlit( Surface *dest, const Rect &from, short dx, short dy ) const {
  if ( s_surface == 0 ) return -1;

  SDL_Rect src, dst;

  src.x = from.x;
  src.y = from.y;
  src.w = from.w;
  src.h = from.h;

  dst.x = dx;
  dst.y = dy;
  dst.w = from.w;
  dst.h = from.h;
  return SDL_LowerBlit( s_surface, &src, dest->s_surface, &dst );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::SetClipRect
// DESCRIPTION: Set the clipping rectangle for the surface.
// PARAMETERS : clip - clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Surface::SetClipRect( const Rect &clip ) const {
  SDL_Rect rect;
  rect.x = clip.LeftEdge();
  rect.y = clip.TopEdge();
  rect.w = clip.Width();
  rect.h = clip.Height();

  SDL_SetClipRect( s_surface, &rect );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::GetClipRect
// DESCRIPTION: Get the current clipping rectangle for the surface.
// PARAMETERS : clip - rect to hold the current clipping rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Surface::GetClipRect( Rect &clip ) const {
  SDL_Rect rect;
  SDL_GetClipRect( s_surface, &rect );
  clip = Rect( rect.x, rect.y, rect.w, rect.h );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::FillRect
// DESCRIPTION: Draw a solid rectangle.
// PARAMETERS : x   - left edge of rectangle
//              y   - top edge of rectangle
//              w   - width of rectangle
//              h   - height of rectangle
//              col - color of rectangle
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::FillRect( short x, short y, unsigned short w, unsigned short h,
                       unsigned long col ) const {
  if ( s_surface == 0 ) return -1;
 
  SDL_Rect src;
  src.x = x;
  src.y = y;
  src.w = w;
  src.h = h;

  return SDL_FillRect( s_surface, &src, col );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::FillRectAlpha
// DESCRIPTION: Draw a rectangle using alpha-blending.
// PARAMETERS : x     - left edge of rectangle
//              y     - top edge of rectangle
//              w     - width of rectangle
//              h     - height of rectangle
//              col   - color of rectangle
//              alpha - alpha value to use for blending (default 128)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::FillRectAlpha( short x, short y, unsigned short w, unsigned short h,
                            const Color &col, unsigned char alpha /* = 128 */ ) const {
  Uint8 *bits, bpp;
  SDL_PixelFormat *fmt = s_surface->format;

  // get offset into surface buffer
  bpp = fmt->BytesPerPixel;
  if ( bpp < 2 ) return FillRect( x, y, w, h, col );   // not supported

  SurfaceLock lck( this );

  for ( int py = y; py < y + h; ++py ) {
    bits = ((Uint8 *)s_surface->pixels) + py * s_surface->pitch + x * bpp;

    for ( int px = x; px < x + w; ++px ) {
      unsigned char r, g, b;
      Uint32 pixel = 0;

      // get pixel from surface
      switch ( bpp ) {
        case 2:
          pixel = *((Uint16 *)(bits));
          break;
        case 3:
          if ( SDL_BYTEORDER == SDL_LIL_ENDIAN )
            pixel = bits[0] + (bits[1] << 8) + (bits[2] << 16);
          else
            pixel = (bits[0] << 16) + (bits[1] << 8) + bits[2];
          break;
        case 4:
          pixel = *((Uint32 *)(bits));
          break;
      }

      // get RGB values from pixel
      r = (((pixel&fmt->Rmask)>>fmt->Rshift)<<fmt->Rloss);
      g = (((pixel&fmt->Gmask)>>fmt->Gshift)<<fmt->Gloss);
      b = (((pixel&fmt->Bmask)>>fmt->Bshift)<<fmt->Bloss);

      // blend with alpha
      r = (((col.r-r)*alpha)>>8) + r;
      g = (((col.g-g)*alpha)>>8) + g;
      b = (((col.b-b)*alpha)>>8) + b;
      DrawPixel( px, py, Color( r, g, b ) );

      bits += bpp;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::FillPattern
// DESCRIPTION: Fill an area of the surface with a graphical pattern.
// PARAMETERS : x       - left edge of rectangle
//              y       - top edge of rectangle
//              w       - width of rectangle
//              h       - height of rectangle
//              pattern - pattern image
//              dx      - left edge of the first pattern on the surface
//              dy      - top edge of the first pattern on the surface
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Surface::FillPattern( short x, short y, unsigned short w, unsigned short h,
                           const Image &pattern, short dx, short dy ) {
  Rect clip;
  GetClipRect( clip );
  SetClipRect( Rect( x, y, w, h ) );

  short cx, cy = y - (y - dy) % pattern.Height();

  do {
    cx = x - (x - dx) % pattern.Width();

    while ( cx < x + w ) {
      pattern.Draw( this, cx, cy );
      cx += pattern.Width();
    }

    cy += pattern.Height();
  } while ( cy < y + h );

  SetClipRect( clip );
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::DrawBox
// DESCRIPTION: Draw a bevelled 3-dimensional box.
// PARAMETERS : box  - box position and dimensions
//              type - box type (see surface.h for details)
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Surface::DrawBox( const Rect &box, unsigned short type ) {
  int x1, y1, x2, y2;
  Color light, shadow;

  if ( type & BOX_RECESSED ) {
    light = Color(CF_COLOR_SHADOW);
    shadow = Color(CF_COLOR_HIGHLIGHT);
  } else {
    light = Color(CF_COLOR_HIGHLIGHT);
    shadow = Color(CF_COLOR_SHADOW);
  }

  if ( type & BOX_SOLID ) FillRect( box, Color(CF_COLOR_BACKGROUND) );

  x1 = box.x;
  y1 = box.y;
  x2 = box.x + box.w - 1;
  y2 = box.y + box.h - 1;

  if ( type & BOX_CARVED ) {
    FillRect( x1, y1, 1, y2-y1+1, shadow );
    FillRect( x1, y1, x2-x1+1, 1, shadow );
    FillRect( x2, y1, 1, y2-y1+1, light );
    FillRect( x1, y2, x2-x1+1, 1, light );

    ++x1; ++y1; --x2; --y2;

    FillRect( x1, y1, 1, y2-y1+1, light );
    FillRect( x1, y1, x2-x1+1, 1, light );
    FillRect( x2, y1, 1, y2-y1+1, shadow );
    FillRect( x1, y2, x2-x1+1, 1, shadow );
  } else {
    FillRect( x1, y1, 1, y2-y1, light );
    FillRect( x1, y1, x2-x1, 1, light );
    FillRect( x2, y1, 1, y2-y1+1, shadow );
    FillRect( x1, y2, x2-x1+1, 1, shadow );
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::DrawPixel
// DESCRIPTION: Draw a single pixel in the specified color. This
//              procedure does not check whether the surface needs to
//              be locked to modify it, so the caller must make sure
//              everything is properly set up.
// PARAMETERS : x   - horizontal position in surface
//              y   - vertical position in surface
//              col - color of pixel
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Surface::DrawPixel( short x, short y, const Color &col ) const {
  Uint32 pixel;
  Uint8 *bits, bpp;

  // map color to display
  pixel = MapRGB( col );

  // get offset into surface buffer
  bpp = s_surface->format->BytesPerPixel;
  bits = ((Uint8 *)s_surface->pixels) + y * s_surface->pitch + x * bpp;

  switch ( bpp ) {
    case 1:
      *((Uint8 *)(bits)) = (Uint8)pixel;
      break;
    case 2:
      *((Uint16 *)(bits)) = (Uint16)pixel;
      break;
    case 3:
      Uint8 r, g, b;

      r = (pixel>>s_surface->format->Rshift) & 0xFF;
      g = (pixel>>s_surface->format->Gshift) & 0xFF;
      b = (pixel>>s_surface->format->Bshift) & 0xFF;
      *((bits) + s_surface->format->Rshift / 8) = r;
      *((bits) + s_surface->format->Gshift / 8) = g;
      *((bits) + s_surface->format->Bshift / 8) = b;
      break;
    case 4:
      *((Uint16 *)(bits)) = (Uint16)pixel;
  }
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::LoadPalette
// DESCRIPTION: Load the color palette from a data file. The file is
//              supposed to be opened already.
// PARAMETERS : file - data file descriptor
//              num  - number of colors to read
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::LoadPalette( MemBuffer &file, unsigned short num ) {
  unsigned char *pal, *col;

  // file contains num colors
  pal = new unsigned char[num * 3];
  if ( !pal ) return -1;
  col = pal;

  file.Read( pal, num * 3 );

  for ( int i = 0; i < num; ++i ) {
    s_surface->format->palette->colors[i].r = *col++;
    s_surface->format->palette->colors[i].g = *col++;
    s_surface->format->palette->colors[i].b = *col++;
  }

  delete [] pal;
  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::LoadImageData
// DESCRIPTION: Load image data from a data file. The file is supposed
//              to be opened already.
// PARAMETERS : file      - data file descriptor
//              hwsurface - tell SDL to try to put the image into video
//                          memory (defaults to false)
// RETURNS    : 0 on success, -1 on error
////////////////////////////////////////////////////////////////////////

int Surface::LoadImageData( MemBuffer &file, bool hwsurface /* = false */ ) {
  unsigned char bpp, flags;
  unsigned short width, height, colors;
  Color ckey;

  width = file.Read16();
  height = file.Read16();
  bpp = file.Read8();   // nothing but 8 supported for now
  flags = file.Read8();
  colors = file.Read16();

  if ( flags & RAW_DATA_TRANSPARENT ) {
    ckey.r = file.Read8();
    ckey.g = file.Read8();
    ckey.b = file.Read8();
  }

  if ( Create( width, height, bpp, (hwsurface ? SDL_HWSURFACE : 0) ) ) return -1;
  if ( LoadPalette( file, colors ) ) return -1;

  for ( int y = 0; y < height; ++y ) {
    if ( file.Read( (Uint8 *)s_surface->pixels + y * s_surface->pitch, width ) != width ) {
      return -1;
    }
  }

  if ( flags & RAW_DATA_TRANSPARENT ) SetColorKey( ckey );

  return 0;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Surface::LoadBMP
// DESCRIPTION: Load a BMP image from a file.
// PARAMETERS : file - BMP file name
// RETURNS    : 0 on success, non-0 on error
////////////////////////////////////////////////////////////////////////

int Surface::LoadBMP( const char *file ) {
  s_surface = SDL_LoadBMP( file );
  if ( s_surface ) {
    w = s_surface->w;
    h = s_surface->h;
  }
  return (s_surface == NULL);
}


////////////////////////////////////////////////////////////////////////
// NAME       : Image::Image
// DESCRIPTION: Create an image from a surface. An image is simply a
//              rectangular portion of the surface.
// PARAMETERS : surface - image surface
//              x       - left edge of image
//              y       - zop edge of image
//              w       - image width
//              h       - image height
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

Image::Image( Surface *surface, short x, short y, unsigned short w,
              unsigned short h ) : Rect( x, y, w, h ) {
  this->surface = surface;
}

////////////////////////////////////////////////////////////////////////
// NAME       : Image::Draw
// DESCRIPTION: Draw the image to a surface.
// PARAMETERS : dest - destination surface
//              x    - left edge of destination rectangle
//              y    - top edge of destination rectangle
// RETURNS    : -
////////////////////////////////////////////////////////////////////////

void Image::Draw( Surface *dest, short x, short y ) const {
  surface->Blit( dest, *this, x, y );
}

