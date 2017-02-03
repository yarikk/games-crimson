localedir = $(pkgdatadir)/locale
locale_DATA = de.dat en.dat fr.dat hu.dat it.dat pl.dat sk.dat sr.dat tr.dat

EXTRA_DIST = $(locale_DATA:.dat=.tmpl)
CLEANFILES = $(locale_DATA)

SUFFIXES = .dat .tmpl
.tmpl.dat:	$(top_builddir)/tools/mklocale
		$(top_builddir)/tools/mklocale -l "$<" "$@"

