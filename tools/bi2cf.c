/*
bi2cf - A map converter for Crimson Fields
copyright (c) 2002      by Florian Dietrich
              2003-2006 by Jens Granseuer

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
// bi2cf.c:
// this file contains the main program and some helper functions
//
// It's far from being perfect, some parts are a little ugly, there's hardly
// any error or boundary checking etc.
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bi2cf.h"

extern const tconvdata bi_convdata;
extern const tconvdata bidd1_convdata;
extern const tconvdata bidd2_convdata;
extern const tconvdata hl_convdata;

#ifdef WIN32
const char *pathdelimiter = "\\";
#else
const char *pathdelimiter = "/";
#endif

const char *mapstrings[5] = {"unknown","BI","BIDD1","BIDD2","HL"};
const char verstr[] = "0.4.9";

const char helpstr[] =
"Usage: %s mapname [options]\n"
"\n"
"(mapname must be the map filename without extension, e.g. \"00\")\n"
"\n"
"Available options:\n"
"   -h               Print this help message\n"
"   -p [1|2]         Set number of players to 1 or 2\n"
"   -f [b|1|2|h]     Force map type to BI / BI Data1 / BI Data2 / HL\n"
"                    (this is always necessary for BI Data1 Maps!)\n"
"   -l               Skip last row and column of the map. This is necessary for\n"
"                    the maps of the PC versions of BI.\n"
"   -t [mapname]     Map title for this map,\n"
"                    default is <maptype>-<mapname>\n"
"   -i [mapinfo]     Map info for this map\n"
"   -o [output file] Writes map to given output file,\n"
"                    by default it becomes the same as mapname + .src\n"
"   -d [map dir]     Directory the map files are in\n"
"   -O [output dir]  Directory to write the output file to\n"
"                    (will be ignored for now if you give an output filename)\n"
"   -q               Don't print progess diagnostics\n"
"   -v               Print version information\n"
"\n"
"Example: %s 01 -p 1 -f 1 -o map2.src -d MAP\n"
"(means: process map 01 in directory \"MAP\", define as 1 player map,\n"
"force map type BI Data1 and write the converted map to \"map2.src\")\n"
"\n";

static int quiet = 0;

// -----------------------------------------------------------------------------
// functions for map file reading
// -----------------------------------------------------------------------------

int uncompress(unsigned char **buffer, int src_size)
// decompress data in buffer
{
    int i,j;
    int dst_size, src_count, dst_count;

    unsigned char *dstbuf = NULL;
    unsigned char *srcbuf = *buffer;
  
    unsigned short copy_num, repeat_num, repeat_offset, temp;

    // little/big endian "detection", not very reliable, but enough for now
    // (should work fine for file sizes up to 65535 bytes, all map files are smaller)
    if ( (srcbuf[4]+srcbuf[5]) > (srcbuf[6]+srcbuf[7]) ) dst_size = (srcbuf[4]) | (srcbuf[5] << 8) | (srcbuf[6] << 16) | (srcbuf[7] << 24);
    else dst_size = (srcbuf[7]) | (srcbuf[6] << 8) | (srcbuf[5] << 16) | (srcbuf[4] << 24);
    dstbuf = malloc(dst_size * sizeof(unsigned char));

    src_count = 8;
    dst_count = 0;

    while( (src_count < src_size) && (dst_count < dst_size) )
    {
        copy_num = srcbuf[src_count++];
        for( i=0 ; i<=7 ; i++)
        {
            if( (dst_count >= dst_size) || (src_count >= src_size) ) break;
            copy_num *= 2;
            if (copy_num > 0xFF)
            {
                copy_num &= 0x00FF;
                temp = srcbuf[src_count++];
                if(src_count >= src_size) break;
                repeat_num = (temp & 0x000F) + 2;
                repeat_offset = srcbuf[src_count++] | ( (temp << 4) & 0x0F00 );
                for( j=0 ; j<=repeat_num ; j++)
                {
                    dstbuf[dst_count] = dstbuf[dst_count-repeat_offset];
                    if(++dst_count >= dst_size) break;
                }
            }
            else dstbuf[dst_count++] = srcbuf[src_count++];
        }
    }

    free(srcbuf);
    *buffer=dstbuf;

    return dst_size;
}

int readfin(FILE *finfile, tmapdata *mapdata)
// Read from XX.FIN file (contains terrain and unit data):
// 1st and 3rd byte are 0x00, 2nd is the map width, 4th map height. After that
// comes one byte of terrain data, one byte of unit data then again one byte of
// terrain data and so on, row by row. HL maps actually have an uneven width
// and height, but there's always one more terrain & unit byte (0xAE 0xFF)
// added at the end of each row, and at the end of the file there is a whole
// row of these, so width & height become even.
{
    int i;
    int maptype = MAP_NONE;
    int highest = 0;

    // get fin file size, allocate mem and read the whole file into the buffer
    fseek(finfile,0,SEEK_END);
    mapdata->finsize = ftell(finfile);
    if( (mapdata->rawfin = (unsigned char *) malloc( mapdata->finsize * sizeof(unsigned char) ) ) == NULL) return 0;
    fseek(finfile,0,SEEK_SET);
    fread(mapdata->rawfin,mapdata->finsize,1,finfile);

    // uncompressing the file if it's TPWM compressed
    if( (mapdata->rawfin[0]=='T') && (mapdata->rawfin[1]=='P') && (mapdata->rawfin[2]=='W') && (mapdata->rawfin[3]=='M') )
    {
        if(!quiet) printf("FIN file is compressed, decompressing...\n");
        mapdata->finsize = uncompress(&mapdata->rawfin,mapdata->finsize);
    }

    // make pointer to actual map data, get x and y size
    mapdata->map = (tfindata *)(mapdata->rawfin + 4);
    mapdata->mapx = mapdata->rawfin[1];
    mapdata->mapy = mapdata->rawfin[3];
    mapdata->mapsize = mapdata->mapx * mapdata->mapy;

    // Find out map type from highest terrain value used:
    // There's always a 0xAE terrain in HL maps, never anything higher than
    // 0x67 in BI and BIDD1 maps, and at least for the BIDD2 standard maps the
    // highest value is always in between those two.
    maptype = MAP_BIDD2;
    for ( i=0 ; i<mapdata->mapsize ; i++ ) if( mapdata->map[i].terrain > highest ) highest = mapdata->map[i].terrain;
    if( highest >= 0xAE ) maptype = MAP_HL;
    if( highest <= 0x67 ) maptype = MAP_BI;

    return maptype;
}

int readshp(FILE *shpfile, tmapdata *mapdata, int maxunits)
// Read from XX.SHP file (contains data about buildings, units in buildings and
// units in transports):
// First come as many bytes as there are different units in the game, if they
// are 0x00, the according unit can be built in factories, if they are 0x01 it
// can't be built. Then comes a byte with the number of records in this file.
// Those records then follow, each being 12 bytes long and carrying information
// about buildings and units inside buildings and transports. See bi2cf.h
// (->tshpdata) for the record's exact format.
{
    // get shp file size, allocate mem and read the whole file into the buffer
    fseek(shpfile,0,SEEK_END);
    mapdata->shpsize = ftell(shpfile);
    if( (mapdata->rawshp = (unsigned char *) malloc( mapdata->shpsize * sizeof(unsigned char) ) ) == NULL) return 0;
    fseek(shpfile,0,SEEK_SET);
    fread(mapdata->rawshp,mapdata->shpsize,1,shpfile);

    // uncompressing the file if it's TPWM compressed
    if( (mapdata->rawshp[0]=='T') && (mapdata->rawshp[1]=='P') && (mapdata->rawshp[2]=='W') && (mapdata->rawshp[3]=='M') )
    {
        if(!quiet) printf("SHP file is compressed, decompressing...\n");
        mapdata->shpsize = uncompress(&mapdata->rawshp,mapdata->shpsize);
    }

    // get number of building records, make pointers to building infos
    mapdata->numbuildings = mapdata->rawshp[maxunits];
    mapdata->canbuild = mapdata->rawshp;
    mapdata->buildings = (tshpdata *)(mapdata->rawshp + maxunits + 1);

    return 1;
}

// -----------------------------------------------------------------------------
// helper functions
// -----------------------------------------------------------------------------

void putunit(FILE *myfile, tconvdata convdata, int xpos, int ypos, unsigned char type, int player, int id)
// puts a specified unit at the specified place
{
    // if type is 0xFF, there's no unit at this place
    if ( type < 0xFF )
    {
        fprintf(myfile,"[unit]\n");
        fprintf(myfile,"type = %s\n",convdata.units[type]);
        fprintf(myfile,"player = %d\n",player);
        fprintf(myfile,"id = %d\n",id);
        fprintf(myfile,"pos = %d/%d\n",xpos,ypos);
        fprintf(myfile,"\n");
    }
}

void putbuilding(FILE *myfile, tconvdata convdata, tmapdata mapdata, int xpos, int ypos, unsigned char type, int crystals, int id)
// puts a specified building at the specified place
{
    int i;
    int mining = 0;

    // another fix for HL: crystals are actually mining.
    // move AND attack turns count for mining, so it is doubled.
    if( convdata.type == MAP_HL ) { mining = crystals*2; crystals = 0; }

    fprintf(myfile,"[building]\n");

    // write name of building
    if( (type+1)/3 == 0 )      fprintf(myfile,"name = 5\n"); /* HQ */
    else if( (type+1)/3 == 1 ) fprintf(myfile,"name = 6\n"); /* Factory */
    else                       fprintf(myfile,"name = 7\n"); /* Depot */

    // all buildings are workshops (can repair units)
    fprintf(myfile,"type = workshop\n");

    // make factories factories and set units you can build in them
    if( (type+1)/3 == 1 )
    {
        fprintf(myfile,"type = factory\n");
        for( i=0 ; i<convdata.maxunits ; i++ )
        {
            if( (mapdata.canbuild[i] == 0) && (strlen(convdata.units[i]) > 0) ) fprintf(myfile,"factory = %s\n",convdata.units[i]);
        }
    }

    // all the other properties  
    if( mining > 0 )
    {
        fprintf(myfile,"mining = %d\n",mining);
    }
    fprintf(myfile,"crystals = %d\n",crystals);
    fprintf(myfile,"player = %i\n",(type+1)%3);
    fprintf(myfile,"id = %d\n",id);
    fprintf(myfile,"pos = %d/%d\n",xpos,ypos);
    fprintf(myfile,"\n");
}

unsigned char getcrystals(tmapdata mapdata, unsigned char type, unsigned char index)
// gets the crystal amounts from shp data for specified building
{
    int i;
    for ( i=0 ; i<mapdata.numbuildings ; i++ )
    {
        if( ( mapdata.buildings[i].type == type ) && ( mapdata.buildings[i].index == index ) ) return mapdata.buildings[i].crystals;
    }
    return 0;
}

void getsubunits(FILE *myfile, tconvdata convdata, tmapdata mapdata, int xpos, int ypos, unsigned char type, int player, unsigned char index, int *unit_id)
// gets and writes the units inside other buildings or units
{
    int i,j;

    for ( i=0 ; i < mapdata.numbuildings ; i++ )
    {
        if( ( mapdata.buildings[i].type == type ) && ( mapdata.buildings[i].index == index ) )
        {
            for ( j=0 ; j<7 ; j++ )
            {
                if ( mapdata.buildings[i].units[j] != 0xFF )
                {
                    fprintf(myfile,"# the following unit is inside a building or transport unit\n");
                    putunit(myfile, convdata, xpos, ypos, mapdata.buildings[i].units[j], player, (*unit_id)++);
                }
            }
            // should probably already return here? but completing doesn't take long anyways
        }
    }
}

// -----------------------------------------------------------------------------
// all the other writing and conversion functions
// -----------------------------------------------------------------------------

void write_generalinfo(FILE *myfile, tmapdata mapdata, tmapinfo mapinfo, int skip)
// writes all the general info stuff into the src file
{
    // write mission stuff
    fprintf(myfile,"[mission]\n");
    fprintf(myfile,"name = 2\n");
    fprintf(myfile,"players = %d\n",mapinfo.players);
    fprintf(myfile,"info = 1\n");
    if( mapinfo.tileset != NULL ) fprintf(myfile,"tileset = %s\n",mapinfo.tileset);
    if( mapinfo.unitset != NULL ) fprintf(myfile,"unitset = %s\n",mapinfo.unitset);

    // little fix for HL maps, their width and height is always one less
    if( skip > 0 )
    {
        fprintf(myfile,"mapwidth = %d\n",mapdata.mapx-1);
        fprintf(myfile,"mapheight = %d\n",mapdata.mapy-1);
    }
    else
    {
        fprintf(myfile,"mapwidth = %d\n",mapdata.mapx);
        fprintf(myfile,"mapheight = %d\n",mapdata.mapy);
    }
    fprintf(myfile,"\n");

    // briefing for both players
    fprintf(myfile,"[player]\n");
    fprintf(myfile,"name = 3\n");
    fprintf(myfile,"briefing = 0\n");
    fprintf(myfile,"\n");
    fprintf(myfile,"[player]\n");
    fprintf(myfile,"name = 4\n");
    fprintf(myfile,"briefing = 0\n");
    fprintf(myfile,"\n");

    // messages (briefing and info);
    fprintf(myfile,"[messages(en)]\n");
    fprintf(myfile,"Destroy all enemy units or capture enemy headquarters!\n");
    fprintf(myfile,"%%\n");
    fprintf(myfile,"%s\n",mapinfo.title);
    fprintf(myfile,"%s\n",mapinfo.info);
    fprintf(myfile,"This map was automatically created by bi2cf %s.\n",verstr);
    fprintf(myfile,"%%\n");
    fprintf(myfile,"%s\n",mapinfo.title);
    fprintf(myfile,"%%\n");
    fprintf(myfile,"Player 1\n");
    fprintf(myfile,"%%\n");
    fprintf(myfile,"Player 2\n");
    fprintf(myfile,"%%\n");
    fprintf(myfile,"Headquarters\n");
    fprintf(myfile,"%%\n");
    fprintf(myfile,"Factory\n");
    fprintf(myfile,"%%\n");
    fprintf(myfile,"Depot\n");
    fprintf(myfile,"[/messages]\n");
    fprintf(myfile,"\n");
}


void write_rawmap(FILE *myfile, tconvdata convdata, tmapdata mapdata, int skip)
// writes the translated map in raw format to src file
{
    int x,y;

    // little fixes for HL, don't write last tile & last row
    int maxx = mapdata.mapx, maxy = mapdata.mapy;
    if( skip > 0 ) { maxx--; maxy--; }

    // read the terrain row by row and write converted map data to file
    fprintf(myfile,"[map-raw]\n");
    for( y=0; y<maxy ; y++ )
    {
        for ( x=0 ; x<maxx ; x++ ) fprintf(myfile,"%3.3d,",convdata.rawtiles[ mapdata.map[y*mapdata.mapx+x].terrain ]);
        fprintf(myfile,"\n");
    }
    fprintf(myfile,"\n");
}


void write_buildings(FILE* myfile, tconvdata convdata, tmapdata mapdata, int *unit_id, int hqid[])
// find the buildings in the map data and place them,
{
    int i,x,y;
    int id = 0;

    // need to count HQs, factories and depots seperately to get units inside
    unsigned char index[3] = { 0, 0, 0 };

    // basically browse through the terrain, see if a tile matches that of a
    // building and then write a building with those x and y coordinates.
    // also write the subunits (units inside buildings) with those coordinates
    for( y=0; y<mapdata.mapy ; y++ )
    {
        for( x=0 ; x<mapdata.mapx ; x++ )
        {
            for( i=0; i<8; i++) if ( mapdata.map[y*mapdata.mapx+x].terrain == convdata.buildings[i] )
            {
                if      (i == 0) hqid[0] = id;
                else if (i == 1) hqid[1] = id;
                putbuilding( myfile, convdata, mapdata, x, y, i, getcrystals(mapdata,(i+1)/3,index[(i+1)/3]), id++ );
                getsubunits( myfile, convdata, mapdata, x, y, (i+1)/3, (i+1)%3, index[(i+1)/3]++, unit_id );
            }
        }
    }
}


void write_units(FILE *myfile, tconvdata convdata, tmapdata mapdata, int *unit_id)
// read units from unit data and place them
{
    int x,y;

    // need to count transports seperately for subunits
    int trans_index = 0;

    // browse through the unit data and place units at the right x/y coordinates
    // when found. also get sub-units when the unit is a transporter.
    for( y=0; y<mapdata.mapy ; y++ )
    {
        for ( x=0 ; x<mapdata.mapx ; x++ )
        {
            if ( (mapdata.map[y*mapdata.mapx+x].unit != 0xFF) && ( strlen( convdata.units[ mapdata.map[y*mapdata.mapx+x].unit/2 ]) > 0) )
            {
                putunit(myfile, convdata, x, y, mapdata.map[y*mapdata.mapx+x].unit/2, (mapdata.map[y*mapdata.mapx+x].unit%2)+1, (*unit_id)++);
                if ( convdata.transport[ mapdata.map[y*mapdata.mapx+x].unit/2 ] > 0 ) getsubunits( myfile, convdata, mapdata, x, y, 3, (mapdata.map[y*mapdata.mapx+x].unit%2)+1, trans_index++,unit_id);
            }
        }
    }
}


void write_events(FILE *myfile, int hqid[])
// writes all the events to file
{
    int i, id = 0;

    for( i=1; i<=2; i++)
    {
        // player i gets enemy hq
        fprintf(myfile,"[event]\n");
        fprintf(myfile,"id = %d\n", id++);
        fprintf(myfile,"type = score\n");
        fprintf(myfile,"player = %d\n", i);
        fprintf(myfile,"trigger = havebuilding\n");
        fprintf(myfile,"tbuilding = %d\n",hqid[2-i]);
        fprintf(myfile,"towner = %d\n", i);
        fprintf(myfile,"ttime = -1\n");
        fprintf(myfile,"success = 100\n\n");

        // player i destroys all enemy units
        fprintf(myfile,"[event]\n");
        fprintf(myfile,"id = %d\n", id++);
        fprintf(myfile,"type = score\n");
        fprintf(myfile,"player = %d\n", i);
        fprintf(myfile,"trigger = unitdestroyed\n");
        fprintf(myfile,"tunit = -1\n");
        fprintf(myfile,"success = 100\n\n");
    
        // show briefing at the beginning
        fprintf(myfile,"[event]\n");
        fprintf(myfile,"id = %d\n", id++);
        fprintf(myfile,"type = message\n");
        fprintf(myfile,"player = %d\n", i);
        fprintf(myfile,"trigger = timer\n");
        fprintf(myfile,"ttime = 0\n");
        fprintf(myfile,"message = 0\n\n");
    }
}

// -----------------------------------------------------------------------------
// cleanung up
// -----------------------------------------------------------------------------

void cleanup(tmapfiles mapfiles, tmapinfo mapinfo, tmapdata mapdata)
// clean up allocated memory; NULL comparison not necessarily needed 
{
    if( mapdata.rawfin != NULL )       free( mapdata.rawfin );
    if( mapdata.rawshp != NULL )       free( mapdata.rawshp );

    if( mapinfo.number != NULL )       free( mapinfo.number );
    if( mapinfo.title != NULL )        free( mapinfo.title );
    if( mapinfo.info != NULL )         free( mapinfo.info );
    if( mapinfo.tileset != NULL )      free( mapinfo.tileset );
    if( mapinfo.unitset != NULL )      free( mapinfo.unitset );

    if( mapfiles.shpfilename != NULL ) free( mapfiles.shpfilename );
    if( mapfiles.srcfilename != NULL ) free( mapfiles.srcfilename );
    if( mapfiles.finfilename != NULL ) free( mapfiles.finfilename );
    if( mapfiles.inputdir != NULL )    free( mapfiles.inputdir );
    if( mapfiles.outputdir != NULL )   free( mapfiles.outputdir );
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    // -------------------------------------------------------------------------
    // variables
    // -------------------------------------------------------------------------

    int i;

    int quiet = 0;
    int maptype = MAP_NONE;
    int forcedmaptype = MAP_NONE;

    tmapfiles mapfiles = { NULL,NULL,NULL,NULL,NULL };
    tmapinfo mapinfo = { 1,NULL,NULL,NULL,NULL,NULL };
    tmapdata mapdata = { NULL,NULL,0,0,0,0,NULL,NULL,NULL,0,0 };

    tconvdata convdata;

    FILE *finfile;
    FILE *shpfile;
    FILE *srcfile;

    int unit_id = 0;
    int hqid[2];
    int skiplast = 0;

    // -------------------------------------------------------------------------
    // print standard option stuff
    // -------------------------------------------------------------------------

    if ( argc<=1 ) { printf(helpstr,argv[0],argv[0]); return 1; }
  
    // -------------------------------------------------------------------------
    // parse command line arguments, not quite perfect yet
    // -------------------------------------------------------------------------

    for (i=1; i<argc; i++)
    {
        if (argv[i][0] == '-') switch(argv[i][1])
        {
            case 'h': // print help
                printf(helpstr,argv[0],argv[0]);
                cleanup(mapfiles,mapinfo,mapdata); 
                return 0;
                break;
            case 'l': // skip last map row + column
                skiplast = 1;
                break;
            case 'p': // number of players (needs work so it gives an error on something different
                if( strcmp(argv[i+1],"1") == 0 )      mapinfo.players = 1;
                else if( strcmp(argv[i+1],"2") == 0 ) mapinfo.players = 2;
                i++;
                break;
            case 'f': // force map type (same as above)
                if( strcmp(argv[i+1],"b") == 0 )      forcedmaptype = MAP_BI;
                else if( strcmp(argv[i+1],"1") == 0 ) forcedmaptype = MAP_BIDD1;
                else if( strcmp(argv[i+1],"2") == 0 ) forcedmaptype = MAP_BIDD2;
                else if( strcmp(argv[i+1],"h") == 0 ) forcedmaptype = MAP_HL;
                i++;
                break;
            case 't': // map title
                mapinfo.title = (char *) malloc( (strlen(argv[i+1])+1) * sizeof(char) );
                strcpy(mapinfo.title, argv[++i]);
                break;
            case 'i': // map info
                mapinfo.info = (char *) malloc( (strlen(argv[i+1])+1) * sizeof(char) );
                strcpy(mapinfo.info, argv[++i]);
                break;
            case 'o': // output file
                mapfiles.srcfilename = (char *) malloc( (strlen(argv[i+1])+1) * sizeof(char) );
                strcpy(mapfiles.srcfilename, argv[++i]);
               break;
            case 'd': // input dir, when malloc'ing leave space for path delimiter (might be needed and makes things easier)
                mapfiles.inputdir = (char *) malloc( (strlen(argv[i+1])+2) * sizeof(char) );
                strcpy(mapfiles.inputdir, argv[++i]);
                break;
            case 'O': // output dir, when malloc'ing leave space for path delimiter (might be needed and makes things easier)
                mapfiles.outputdir = (char *) malloc( (strlen(argv[i+1])+2) * sizeof(char) );
                strcpy(mapfiles.outputdir, argv[++i]);
                break;
            case 'v': // version information
                printf("bi2cf %s\n",verstr);
                cleanup(mapfiles,mapinfo,mapdata); 
                return 0;
                break;
            case 'q': // quiet
                quiet = 1;
                break;
            default:
                fprintf(stderr,"ERROR: Unknown argument -%c\n",argv[i][1]);
                cleanup(mapfiles,mapinfo,mapdata);
                return 1;
                break;
        }
        else
        {
            mapinfo.number = (char *) malloc( (strlen(argv[i])+2) * sizeof(char) );
            strcpy(mapinfo.number,argv[i]);
        }
    }

    if( mapinfo.number == NULL )
    {
        fprintf(stderr,"ERROR: You must give a map name\n");
        cleanup(mapfiles,mapinfo,mapdata);
        return 1;
    }

    // -------------------------------------------------------------------------
    // fill the missing strings for the input files
    // -------------------------------------------------------------------------

    if( mapfiles.inputdir == NULL )
    {
        mapfiles.inputdir = (char *) malloc( 2 * sizeof(char) );
        strcpy(mapfiles.inputdir,"");
    }
    else if( mapfiles.inputdir[strlen(mapfiles.inputdir)-1] != pathdelimiter[0] ) strcat(mapfiles.inputdir,pathdelimiter);

    i = strlen(mapinfo.number) + strlen(mapfiles.inputdir) + 5;
    mapfiles.finfilename = (char *) malloc(i);
    strcpy(mapfiles.finfilename,mapfiles.inputdir);
    strcat(mapfiles.finfilename,mapinfo.number);
    strcat(mapfiles.finfilename,".FIN");

    mapfiles.shpfilename = (char *) malloc(i);
    strcpy(mapfiles.shpfilename,mapfiles.inputdir);
    strcat(mapfiles.shpfilename,mapinfo.number);
    strcat(mapfiles.shpfilename,".SHP");

    // -------------------------------------------------------------------------
    // read data from the FIN file, will also return map type;
    // -------------------------------------------------------------------------

    if(!quiet) printf("Reading FIN file %s ...\n",mapfiles.finfilename);
    finfile = fopen(mapfiles.finfilename,"rb");
    if ( !finfile )
    {
        mapfiles.finfilename[i-4] = '\0';
        strcat(mapfiles.finfilename, "fin");
        finfile = fopen( mapfiles.finfilename,"rb");
    }

    if ( !finfile )
    {
        fprintf(stderr,"ERROR: Could not open FIN file %s for reading\n",mapfiles.finfilename);
        cleanup(mapfiles,mapinfo,mapdata);
        return 1;
    }
    maptype = readfin(finfile,&mapdata);
    fclose(finfile);

    // -------------------------------------------------------------------------
    // see if we're forcing a specific map type, assign correct conversion data
    // -------------------------------------------------------------------------

    if( forcedmaptype != MAP_NONE )
    {
        if(!quiet)
        {
            printf("Forcing use of map type %s (type %s was detected)\n"
                   "Always make sure you're forcing the use of the correct map type.\n",
                   mapstrings[forcedmaptype], mapstrings[maptype] );
        }
        maptype = forcedmaptype;
    }
    else
    {
        if (!quiet)
        {
            printf("Using detected map type %s\n",mapstrings[maptype]);
            if( maptype == MAP_BI )
            {
                printf("IMPORTANT: If you are converting BI Data1 maps, they get detected as BI maps.\n");
                printf("           For the conversion to work properly you must force the use of the\n");
                printf("           correct map type (%s) with the option \"-f 1\".\n",mapstrings[MAP_BIDD1]);
                printf("           If you know the map type, it is never a bad idea to force it.\n");
            }
        }
    }

    switch(maptype)
    {
        case MAP_BI:
            convdata = bi_convdata;
            break;
        case MAP_BIDD1:
            convdata = bidd1_convdata;
            if(mapdata.rawfin[mapdata.finsize-2] == 0x61) skiplast = 1;
            break;
        case MAP_BIDD2:
            convdata = bidd2_convdata;
            if(mapdata.rawfin[mapdata.finsize-2] == 0x63) skiplast = 1;
            break;
        case MAP_HL:
            convdata = hl_convdata;
            skiplast = 1;
            break;
        default:
            fprintf(stderr,"ERROR: Could not detect map format\n");
            cleanup(mapfiles,mapinfo,mapdata);
            return 1;
            break;
    }

    // -------------------------------------------------------------------------
    // read data from the SHP file
    // -------------------------------------------------------------------------

    if(!quiet) printf("Reading SHP file %s ...\n",mapfiles.shpfilename);
    shpfile = fopen(mapfiles.shpfilename,"rb");
    if ( !shpfile )
    {
        mapfiles.shpfilename[i-4] = '\0';
        strcat(mapfiles.shpfilename, "shp");
        shpfile = fopen( mapfiles.shpfilename,"rb");
    }

    if ( !shpfile  )
    {
        fprintf(stderr,"ERROR: Could not open SHP file %s for reading\n",mapfiles.shpfilename);
        cleanup(mapfiles,mapinfo,mapdata);
        return 1;
    }
    readshp(shpfile,&mapdata,convdata.maxunits);
    fclose(shpfile);

    // -------------------------------------------------------------------------
    // fill all the missing strings for map info and output file
    // -------------------------------------------------------------------------

    if( mapinfo.title == NULL )
    {
        mapinfo.title = (char *) malloc( (strlen(mapinfo.number) + strlen(mapstrings[convdata.type]) + 5) * sizeof(char) );
        strcpy(mapinfo.title,mapstrings[convdata.type]);
        strcat(mapinfo.title,"-");
        strcat(mapinfo.title,mapinfo.number);
    }

    if( mapinfo.info == NULL )
    {
        mapinfo.info = (char *) malloc( (strlen(mapinfo.number) + strlen(mapstrings[convdata.type]) + 50) * sizeof(char) );
        sprintf(mapinfo.info,"This is a conversion of map %s from %s",mapinfo.number,mapstrings[convdata.type]);
    }

    if( mapfiles.outputdir == NULL )
    {
        mapfiles.outputdir = (char *) malloc( 2 * sizeof(char) );
        strcpy(mapfiles.outputdir,"");
    }
    else if( mapfiles.outputdir[strlen(mapfiles.outputdir)-1] != pathdelimiter[0] ) strcat(mapfiles.outputdir,pathdelimiter);

    if( mapfiles.srcfilename == NULL )
    {
        mapfiles.srcfilename = (char *) malloc( (strlen(mapinfo.title)+strlen(mapfiles.outputdir)+5) * sizeof(char) );
        strcpy(mapfiles.srcfilename,mapfiles.outputdir);
        strcat(mapfiles.srcfilename,mapinfo.title);
        strcat(mapfiles.srcfilename,".src");
    }

    // -------------------------------------------------------------------------
    // writing everything to output file, all conversions happen here too
    // -------------------------------------------------------------------------

    if(!quiet) printf("Writing output map file %s ...\n",mapfiles.srcfilename);
    srcfile = fopen(mapfiles.srcfilename,"w");
    if ( !srcfile  )
    {
        fprintf(stderr,"ERROR: Could not open output map file %s for writing\n",mapfiles.srcfilename);
        cleanup(mapfiles,mapinfo,mapdata);
        return 1;
     }

    write_generalinfo (srcfile, mapdata, mapinfo, skiplast);
    write_rawmap (srcfile, convdata, mapdata, skiplast);
    write_buildings   (srcfile, convdata, mapdata, &unit_id, hqid);
    write_units       (srcfile, convdata, mapdata, &unit_id);
    write_events      (srcfile, hqid);

    fclose(srcfile);

    // -------------------------------------------------------------------------
    // that should be it! clean up and exit
    // -------------------------------------------------------------------------

    cleanup(mapfiles,mapinfo,mapdata);
    if(!quiet) printf("Processing complete!\n\n");

    return 0;
}

