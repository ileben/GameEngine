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

noinst_PROGRAMS = TexPainter
TexPainter_CXXFLAGS = @GE_CXXFLAGS@ -I${prefix}/include -I$(top_builddir)/src/ -rdynamic
TexPainter_LDFLAGS = @GE_LDFLAGS@ -L${prefix}/lib -L$(top_builddir)/src/
TexPainter_LDADD = @GE_LDADD@ $(top_builddir)/src/libGameEngine.la
TexPainter_SOURCES = TexPainter.cpp
