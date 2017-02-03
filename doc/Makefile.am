if BUILD_BI2CF
BI2CF_MAN = bi2cf.6
endif
if BUILD_CFED
CFED_MAN = cfed.6
endif
if BUILD_COMET
COMET_MAN = comet.6
endif
man_MANS = $(BI2CF_MAN) $(CFED_MAN) $(COMET_MAN) crimson.6
xml_DOCS = bi2cf.xml cfed.xml comet.xml crimson.xml
EXTRA_DIST = bi2cf.6 cfed.6 comet.6 crimson.6 $(xml_DOCS) crimson.css

XSLT_DB2MAN = http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl
XSLT_DB2HTML = http://docbook.sourceforge.net/release/xsl/current/html/docbook.xsl

%.6:		%.xml
		$(if $(XSLTPROC), \
		  $(XSLTPROC) -o "$@" $(XSLT_DB2MAN) "$<")

%.html:		%.xml
		$(if $(XSLTPROC), \
		  $(XSLTPROC) --stringparam html.stylesheet crimson.css \
		  -o "$@" $(XSLT_DB2HTML) "$<")

html-local:	bi2cf.html cfed.html comet.html crimson.html

check-local:	$(xml_DOCS)
		$(if $(XMLLINT), \
		  cd $(top_srcdir)/doc; $(XMLLINT) --noout --valid $(xml_DOCS))

# require xsltproc on make dist
dist-hook:
		$(if $(XSLTPROC),, \
		  $(error xsltproc must be available in order to make dist))


