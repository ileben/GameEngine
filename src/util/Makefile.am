lib_LTLIBRARIES = libGameUtil.la

GE_UTIL_HEADS =\
	geArrayList.h\
	geArraySet.h\
	geClass.h\
	geDefs.h\
	geHeapArrayList.h\
	geLinkedList.h\
	geLinkedListIter.h\
	geSerialize.h\
	geString.h\
	geTextParser.h\
	geTime.h\
	geUnClass.h\
	geUtil.h

libGameUtil_la_CXXFLAGS = -I${prefix}/include -I$(top_builddir)/src
libGameUtil_la_LDFLAGS = -L${prefix}/lib
libGameUtil_la_SOURCES =\
	${GE_UTIL_HEADS}\
	geArrayList.cpp\
	geClass.cpp\
	geSerialize.cpp\
	geString.cpp\
	geTime.cpp

GameUtil_includedir = $(includedir)/GameEngine/util
GameUtil_include_HEADERS = ${GE_UTIL_HEADS}
