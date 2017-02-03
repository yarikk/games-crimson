/* Crimson Fields -- a game of tactical warfare
   Copyright (C) 2000-2007 Jens Granseuer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
   SDL_zlib.c
   This file provides support for reading and writing zlib-compressed
   files (gzip) using the SDL_RWops interface of SDL.
*/

#include "SDL_rwops.h"
#include "SDL_error.h"

#ifdef HAVE_LIBZ

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <zlib.h>

/* Functions to read/write gzip files */

static int gzip_seek(SDL_RWops *context, int offset, int whence) {
  int pos;

  pos = gzseek((gzFile)context->hidden.unknown.data1, offset, whence);
  if (pos == -1) SDL_Error(SDL_EFSEEK);
  return pos; 
}

static int gzip_read(SDL_RWops *context, void *ptr, int size, int maxnum) {
  size_t nread;

  nread = gzread((gzFile)context->hidden.unknown.data1, ptr, size * maxnum);
  if (nread == -1) SDL_Error(SDL_EFREAD);
  return nread;
}

static int gzip_write(SDL_RWops *context, const void *ptr, int size, int num) {
  size_t nwrote;

  nwrote = gzwrite((gzFile)context->hidden.unknown.data1, ptr, size * num);
  if (nwrote == 0) SDL_Error(SDL_EFWRITE);
  return nwrote;
}

static int gzip_close(SDL_RWops *context) {
  int rc = 0;

  if (context) {
    rc = gzclose((gzFile)context->hidden.unknown.data1);
    SDL_FreeRW(context);
  }
  return rc;
}

/* this comes straight from the SDL sources */
#ifdef macintosh
/*
 * translate unix-style slash-separated filename to mac-style colon-separated
 * name; return malloced string
 */
static char *unix_to_mac(const char *file) {
  flen = strlen(file);
  char *path = malloc(flen + 2);
  const char *src = file;
  char *dst = path;
  if(*src == '/') {
    /* really depends on filesystem layout, hope for the best */
    src++;
  } else {
    /* Check if this is a MacOS path to begin with */
    if(*src != ':')
      *dst++ = ':';   /* relative paths begin with ':' */
  }
  while(src < file + flen) {
    const char *end = strchr(src, '/');
    int len;
    if(!end)
      end = file + flen; /* last component */
    len = end - src;
    if(len == 0 || (len == 1 && src[0] == '.')) {
      /* remove repeated slashes and . */
    } else {
      if(len == 2 && src[0] == '.' && src[1] == '.') {
        /* replace .. with the empty string */
      } else {
        memcpy(dst, src, len);
        dst += len;
      }
      if(end < file + flen)
        *dst++ = ':';
    }
    src = end + 1;
  }
  *dst++ = '\0';
  return path;
}
#endif /* macintosh */
#endif /* HAVE_LIBZ */

/*
 if zlib is not available this function returns a standard file
 structure as created by SDL_RWFromFile
*/
SDL_RWops *SDL_RWFromGzip(const char *file, const char *mode) {
  SDL_RWops *rwops;

#ifdef HAVE_LIBZ
  rwops = SDL_AllocRW();
  if (rwops != NULL) {
    gzFile gzfp;

# ifdef macintosh
    char *mpath = unix_to_mac(file);
    gzfp = gzopen(mpath, mode);
    free(mpath);
# else
    gzfp = gzopen(file, mode);
# endif /* macintosh */

    if (gzfp != NULL) {
      rwops->hidden.unknown.data1 = gzfp;
      rwops->seek = gzip_seek;
      rwops->read = gzip_read;
      rwops->write = gzip_write;
      rwops->close = gzip_close;
    } else {
      SDL_FreeRW(rwops);
      rwops = NULL;
    }
  }

#else
  rwops = SDL_RWFromFile(file, mode);
#endif /* HAVE_LIBZ */

  return rwops;
}

