if BUILD_COMET
bin_PROGRAMS = comet
endif
comet_SOURCES = \
building.cpp building.h \
edwindow.cpp edwindow.h \
eventwindow.cpp eventwindow.h \
extwindow2.cpp extwindow2.h \
gfxwidget.cpp gfxwidget.h \
main.cpp main.h \
map.cpp map.h \
mapgen.cpp mapgen.h \
mission.cpp mission.h \
msgs.h \
uiaux.cpp uiaux.h \
unit.cpp unit.h \
../common/SDL_zlib.c ../common/SDL_zlib.h \
../common/button.cpp ../common/button.h \
../common/color.h \
../common/extwindow.cpp ../common/extwindow.h \
../common/fileio.cpp ../common/fileio.h \
../common/filewindow.cpp ../common/filewindow.h \
../common/font.cpp ../common/font.h \
../common/gamedefs.h \
../common/gamewindow.cpp ../common/gamewindow.h \
../common/globals.h \
../common/hexsup.cpp ../common/hexsup.h \
../common/lang.cpp ../common/lang.h \
../common/list.cpp ../common/list.h \
../common/listselect.cpp ../common/listselect.h \
../common/lset.cpp ../common/lset.h \
../common/mapview.cpp ../common/mapview.h \
../common/misc.cpp ../common/misc.h \
../common/rect.cpp ../common/rect.h \
../common/slider.cpp ../common/slider.h \
../common/sound.cpp ../common/sound.h \
../common/strutil.cpp ../common/strutil.h \
../common/surface.cpp ../common/surface.h \
../common/textbox.cpp ../common/textbox.h \
../common/view.cpp ../common/view.h \
../common/widget.cpp ../common/widget.h \
../common/window.cpp ../common/window.h

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/src/common
LIBS = @LIBS@ @CF_LIBS@
DEFS = @DEFS@ -DCF_DATADIR=\"$(pkgdatadir)/\"

