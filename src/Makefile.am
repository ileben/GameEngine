GE_HEADS =\
	geCamera.h\
	geClass.h\
	geDefs.h\
	geDMeshIters.h\
	geDynamicMesh.h\
	geEngine.h\
	geGLHeaders.h\
	geHmeshAdjiter.h\
	geHmeshDataiter.h\
	geHmesh.h\
	geKernel.h\
	geLoad3ds.h\
	geLoadObj.h\
	geLoader.h\
	geMaterial.h\
	geMatrix.h\
	geObject.h\
	geProperty.h\
	geRenderer.h\
	geResource.h\
	geSaveObj.h\
	geScene.h\
	geShaders.h\
	geShader.h\
	geShape.h\
	geStaticMesh.h\
	geTexture.h\
	geUVMesh.h\
	geVectors.h\
	widgets/geFpsLabel.h\
	widgets/geLabel.h\
	widgets/geWidget.h

lib_LTLIBRARIES = libGameEngine.la
libGameEngine_la_CXXFLAGS = -I${prefix}/include -Winline
libGameEngine_la_LDFLAGS = -L${prefix}/lib
libGameEngine_la_SOURCES =\
	${GE_HEADS}\
	geCamera.cpp\
	geClass.cpp\
	geDMesh.cpp\
	geHMesh.cpp\
	geKernel.cpp\
	geLoad3ds.cpp\
	geLoadObj.cpp\
	geLoadTex.cpp\
	geLoader.cpp\
	geMaterial.cpp\
	geMatrix.cpp\
	geObject.cpp\
	geProperty.cpp\
	geRenderer.cpp\
	geResource.cpp\
	geSaveObj.cpp\
	geScene.cpp\
	geShaders.cpp\
	geShader.cpp\
	geShape.cpp\
	geSMesh.cpp\
	geTexture.cpp\
	geUVMesh.cpp\
	geVectors.cpp\
	widgets/geFpsLabel.cpp\
	widgets/geLabel.cpp\
	widgets/geWidget.cpp

GE_includedir = $(includedir)/GE
GE_include_HEADERS = ${GE_HEADS}
