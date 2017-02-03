if BUILD_BI2CF
inst_bi2cf = bi2cf
endif
if BUILD_CFED
inst_cfed = cfed
else
noinst_cfed = cfed
endif
if BUILD_CF2BMP
inst_cf2bmp = cf2bmp
endif

bin_PROGRAMS = $(inst_bi2cf) $(inst_cfed) $(inst_cf2bmp)
noinst_PROGRAMS = mkdatafile mklocale mktileset mkunitset $(noinst_cfed)

bi2cf_SOURCES = bi2cf.c bi2cf.h bi_data.c bidd1_data.c bidd2_data.c hl_data.c

cfed_SOURCES = cfed.cpp parser.cpp parser.h \
../src/common/SDL_zlib.c \
../src/common/fileio.cpp \
../src/common/lang.cpp \
../src/common/list.cpp \
../src/common/lset.cpp \
../src/common/misc.cpp \
../src/common/rect.cpp \
../src/common/sound.cpp \
../src/common/strutil.cpp \
../src/common/surface.cpp \
../src/comet/building.cpp \
../src/comet/map.cpp \
../src/comet/mission.cpp \
../src/comet/unit.cpp

mklocale_SOURCES = mklocale.cpp \
../src/common/SDL_zlib.c \
../src/common/lang.cpp \
../src/common/fileio.cpp \
../src/common/strutil.cpp

mkunitset_SOURCES = mkunitset.cpp \
mksurface.cpp mksurface.h parser.cpp parser.h \
../src/common/fileio.cpp \
../src/common/lang.cpp \
../src/common/lset.cpp \
../src/common/rect.cpp \
../src/common/sound.cpp \
../src/common/strutil.cpp \
../src/common/surface.cpp \
../src/common/SDL_zlib.c

mktileset_SOURCES = mktileset.cpp \
mksurface.cpp mksurface.h parser.cpp parser.h \
../src/common/SDL_zlib.c \
../src/common/fileio.cpp \
../src/common/lang.cpp \
../src/common/lset.cpp \
../src/common/rect.cpp \
../src/common/sound.cpp \
../src/common/strutil.cpp \
../src/common/surface.cpp

mkdatafile_SOURCES = mkdatafile.cpp mksurface.cpp \
../src/common/SDL_zlib.c \
../src/common/fileio.cpp \
../src/common/surface.cpp

cf2bmp_SOURCES = cf2bmp.cpp \
../src/common/SDL_zlib.c \
../src/common/fileio.cpp \
../src/common/lang.cpp \
../src/common/list.cpp \
../src/common/lset.cpp \
../src/common/mapview.cpp \
../src/common/misc.cpp \
../src/common/rect.cpp \
../src/common/sound.cpp \
../src/common/strutil.cpp \
../src/common/surface.cpp \
../src/comet/building.cpp \
../src/comet/map.cpp \
../src/comet/mission.cpp \
../src/comet/unit.cpp

AM_CPPFLAGS = -DDISABLE_SOUND -I$(top_srcdir)/src/common -I$(top_srcdir)/src/comet
DEFS = @DEFS@ -DCF_DATADIR=\"$(pkgdatadir)/\"

pkgdata_DATA = cf.dat default.tiles default.units
CLEANFILES = $(pkgdata_DATA) 
EXTRA_DIST = default.tsrc default.usrc

gfxdir = $(top_srcdir)/gfx

cf.dat:		mkdatafile $(cfdat_gfx)
		$(top_builddir)/tools/mkdatafile $(gfxdir)/CFIcons.bmp cf.dat

default.tiles:	mktileset default.tsrc $(gfxdir)/CFTiles.bmp
		$(top_builddir)/tools/mktileset $(top_srcdir)/tools/default.tsrc default.tiles $(gfxdir)

default.units:	mkunitset default.usrc $(gfxdir)/CFUnits.bmp
		$(top_builddir)/tools/mkunitset $(top_srcdir)/tools/default.usrc default.units $(gfxdir)

