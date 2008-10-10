lib_LTLIBRARIES = libGameIO.la

GE_IO_HEADS =\
	geFile.h

libGameIO_la_CXXFLAGS = -I${prefix}/include -I$(top_builddir)/src
libGameIO_la_LDFLAGS = -L${prefix}/lib
libGameIO_la_SOURCES =\
	${GE_IO_HEADS}\
	geFile.cpp

GameIO_includedir = $(includedir)/GameEngine/io
GameIO_include_HEADERS = ${GE_IO_HEADS}
