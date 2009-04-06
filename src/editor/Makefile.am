GE_LIBS =\
	$(top_builddir)/src/util/libGameUtil.la \
	$(top_builddir)/src/io/libGameIO.la \
	$(top_builddir)/src/image/libGameImage.la \
	$(top_builddir)/src/engine/libGameEngine.la

noinst_PROGRAMS = GameEditor

GameEditor_CXXFLAGS = @GE_CXXFLAGS@ -I${prefix}/include -I$(top_builddir)/src/
GameEditor_LDFLAGS = @GE_LDFLAGS@ -L${prefix}/lib 
GameEditor_LDADD = @GE_LDADD@ ${GE_LIBS}
GameEditor_SOURCES = GameEditor.cpp

