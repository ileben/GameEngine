lib_LTLIBRARIES = libGameImage.la

GE_IMAGE_HEADS =\
	geImage.h\
	jpeg_external.h\
	png_external.h

libGameImage_la_CXXFLAGS = -I${prefix}/include -I$(top_builddir)/src
libGameImage_la_LDFLAGS = -L${prefix}/lib
libGameImage_la_SOURCES =\
	${GE_IMAGE_HEADS}\
	jpeg_internal.h\
	png_internal.h\
	geImage.cpp\
	jpeg_error.cpp\
	jpeg_read.cpp\
	jpeg_write.cpp\
	png_error.cpp\
	png_read.cpp

GameImage_includedir = $(includedir)/GameEngine/image
GameImage_include_HEADERS = ${GE_IMAGE_HEADS}
