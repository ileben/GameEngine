GE_LIBS =\
	$(top_builddir)/src/util/libGameUtil.la \
	$(top_builddir)/src/io/libGameIO.la \
	$(top_builddir)/src/image/libGameImage.la \
	$(top_builddir)/src/engine/libGameEngine.la

EXTRA_DIST =\
  texture.jpg\
  specularity.jpg\
  specularity.vertex.c\
  specularity.fragment.c\
  zekko.obj\
  venusm.obj\
  plane.obj\
  plane2.obj\
  hmeshtest.cpp\
  showcase.cpp \
  test.cpp

noinst_PROGRAMS = TexPainter TestSkin

TexPainter_CXXFLAGS = @GE_CXXFLAGS@ -I${prefix}/include -I$(top_builddir)/src/
TexPainter_LDFLAGS = @GE_LDFLAGS@ -L${prefix}/lib 
TexPainter_LDADD = @GE_LDADD@ ${GE_LIBS}
TexPainter_SOURCES = TexPainter.cpp

TestSkin_CXXFLAGS = @GE_CXXFLAGS@ -I${prefix}/include -I$(top_builddir)/src/
TestSkin_LDFLAGS = @GE_LDFLAGS@ -L${prefix}/lib
TestSkin_LDADD = @GE_LDADD@ ${GE_LIBS}
TestSkin_SOURCES = TestSkin.cpp
