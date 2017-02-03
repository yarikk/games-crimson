/*
bi2cf - A map converter for Crimson Fields
copyright (c) 2002 by Florian Dietrich

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

//-----------------------------------------------------------------------------
// bi2cf.h:
// header file for bi2cf, some data types etc.
//
// not much to say about this...
//-----------------------------------------------------------------------------

#define MAP_NONE 0
#define MAP_BI 1
#define MAP_BIDD1 2
#define MAP_BIDD2 3
#define MAP_HL 4

// this will hold the properties of a building
typedef struct {
    unsigned char owner;    // owner of buildung/unit [0|1|2]
    unsigned char type;     // type [0=HQ|1=Factory|2=Depot|3=Unit]
    unsigned char index;    // xth unit/building of type
    unsigned char crystals; // number of crystals inside
    unsigned char unknown;  // probably max capacity of crystals?
    unsigned char units[7]; // units inside building/unit
} tshpdata;

// this will hold all the game-specific conversion data
typedef struct {
    int type;
    const unsigned char (*units)[20];
    const unsigned short *rawtiles;
    const unsigned char *buildings;
    const unsigned char *transport;
    int maxunits;
} tconvdata;

// this will hold terrain and unit information of a field
typedef struct {
    unsigned char terrain;
    unsigned char unit;
} tfindata;

// this will hold all of the map data read from the files
typedef struct {
    unsigned char *rawfin;
    tfindata *map;
    int mapx, mapy, mapsize, finsize;
    unsigned char *rawshp;
    unsigned char *canbuild;
    tshpdata *buildings;
    int numbuildings, shpsize;
} tmapdata;

// this will hold all the filenames and directories
typedef struct {
    char *inputdir;
    char *outputdir;
    char *srcfilename;
    char *finfilename;
    char *shpfilename;
} tmapfiles;

// this will hold all the map information given by the user
typedef struct {
    int players;
    char *number;
    char *title;
    char *info;
    char *tileset;
    char *unitset;
} tmapinfo;

