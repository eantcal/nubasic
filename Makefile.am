bin_PROGRAMS = nubasic

nubasic_SOURCES = config.h main.cc

if COMPILE_TINYVER

AM_CXXFLAGS = $(INTI_CFLAGS) -std=c++11 -O3 -DTINY_NUBASIC_VER -I$(top_srcdir)/include
nubasic_LDADD = $(INTI_LIBS) lib/libnubasic.a

else

AM_CXXFLAGS = $(INTI_CFLAGS) -pthread -std=c++11 -O3 -I$(top_srcdir)/include
nubasic_LDADD = -lpthread $(INTI_LIBS) lib/libnubasic.a

endif 

SUBDIRS=lib
if COMPILE_GTKIDE
SUBDIRS+=ide/gtk/mipjson/miptknzr/lib ide/gtk/mipjson/lib ide/gtk
endif

EXTRA_DIST=\
   nubasic.sln \
   nubasic.vcxproj \
   include \
   examples \
   miptknzr/include \
   mipjson/include \
   ide/gtk/*.h \
   ide/win/*.cpp \
   ide/win/*.h \
   ide/win/*.sln \
   ide/win/*.vcxproj \
   ide/win/*.rc \
   ide/win/*.ico \
   ide/win/*.rc \
   ide/win/*.cc \
   ide/scintilla/bin \
   ide/scintilla/doc \
   ide/scintilla/gtk \
   ide/scintilla/cocoa \
   ide/scintilla/include \
   ide/scintilla/lexers \
   ide/scintilla/lexlib \
   ide/scintilla/qt \
   ide/scintilla/scripts \
   ide/scintilla/src \
   ide/scintilla/test \
   ide/scintilla/CONTRIBUTING \
   ide/scintilla/cppcheck.suppress \
   ide/scintilla/delbin.bat \
   ide/scintilla/.hg* \
   ide/scintilla/License.txt \
   ide/scintilla/README \
   ide/scintilla/tgzsrc \
   ide/scintilla/version.txt \
   ide/scintilla/zipsrc.bat \
   ide/scintilla/win32/*.cxx \
   ide/scintilla/win32/*.h \
   ide/scintilla/win32/*.sln \
   ide/scintilla/win32/*.vcxproj \
   ide/scintilla/win32/*.def \
   ide/scintilla/win32/*.mak \
   ide/scintilla/win32/*.rc \
   ide/scintilla/win32/*.properties
