bin_PROGRAMS = nubasicide

nubasicide_SOURCES = config.h \
   ideres.cc \
   nubasicide.cc 

AM_CXXFLAGS = \
   $(INTI_CFLAGS) \
   $(NUBASICIDE_CFLAGS) \
   -I$(top_srcdir)//include \
   -I$(top_srcdir)/ide/scintilla/include \
   -I$(top_srcdir)/miptknzr/include \
   -I$(top_srcdir)/mipjson/include \
   -pthread \
   -std=c++14 \
   -DGTK 

nubasicide_LDADD = \
    $(top_builddir)/lib/libnubasic.a \
    $(top_builddir)/ide/scintilla/bin/scintilla.a \
    $(top_builddir)/mipjson/lib/libmipjson.a \
    $(top_builddir)/miptknzr/lib/libmiptknzr.a \
    -lpthread \
    $(INTI_LIBS) \
    $(NUBASICIDE_LIBS) 

