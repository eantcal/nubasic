bin_PROGRAMS = nubasic

nubasic_SOURCES = config.h main.cc

if COMPILE_TINYVER

AM_CXXFLAGS = $(INTI_CFLAGS) -std=c++11 -DTINY_NUBASIC_VER -I$(top_srcdir)/include
nubasic_LDADD = $(INTI_LIBS) lib/libnubasic.a

else

AM_CXXFLAGS = $(INTI_CFLAGS) -pthread -std=c++11 -I$(top_srcdir)/include
nubasic_LDADD = -lpthread $(INTI_LIBS) lib/libnubasic.a

endif 

SUBDIRS=lib
if COMPILE_GTKIDE
SUBDIRS+=miptknzr/lib mipjson/lib ide/gtk
endif

EXTRA_DIST=examples ide/win include miptknzr/include mipjson/include ide/gtk/*.h ide/scintilla nubasic.sln nubasic.vcxproj
