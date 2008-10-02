GE_HEADS =\
	geActor.h\
	geCamera.h\
	geClass.h\
	geDefs.h\
	geDynArrayList.h\
	geEngine.h\
	geGLHeaders.h\
	geHmesh.h\
	geHmeshAdjiter.h\
	geHmeshDataiter.h\
	geKernel.h\
	geLoad3ds.h\
	geLoadObj.h\
	geLoader.h\
	geMaterial.h\
	geMatrix.h\
	geObject.h\
	gePolyMesh.h\
	gePolyMeshIters.h\
	gePolyMeshActor.h\
	geProperty.h\
	geRenderer.h\
	geResource.h\
	geSaveObj.h\
	geScene.h\
	geSerialize.h\
	geShaders.h\
	geShader.h\
	geSkinAnim.h\
	geSkinMesh.h\
	geSkinPose.h\
	geTexture.h\
	geTexMesh.h\
	geTriMesh.h\
	geTriMeshActor.h\
	geUnClass.h\
	geUtil.h\
	geVectors.h\
	widgets/geFpsLabel.h\
	widgets/geLabel.h\
	widgets/geWidget.h

lib_LTLIBRARIES = libGameEngine.la
libGameEngine_la_CXXFLAGS = -I${prefix}/include
libGameEngine_la_LDFLAGS = -L${prefix}/lib
libGameEngine_la_SOURCES =\
	${GE_HEADS}\
	geActor.cpp\
	geCamera.cpp\
	geClass.cpp\
	geDynArrayList.cpp\
	geHMesh.cpp\
	geKernel.cpp\
	geLoad3ds.cpp\
	geLoadObj.cpp\
	geLoadTex.cpp\
	geLoader.cpp\
	geMaterial.cpp\
	geMatrix.cpp\
	geObject.cpp\
	gePolyMesh.cpp\
	gePolyMeshActor.cpp\
	geProperty.cpp\
	geRenderer.cpp\
	geResource.cpp\
	geSaveObj.cpp\
	geScene.cpp\
	geSerialize.cpp\
	geShaders.cpp\
	geShader.cpp\
	geSkinAnim.cpp\
	geSkinMesh.cpp\
	geSkinPose.cpp\
	geTexture.cpp\
	geTexMesh.cpp\
	geTriMesh.cpp\
	geTriMeshActor.cpp\
	geVectors.cpp\
	widgets/geFpsLabel.cpp\
	widgets/geLabel.cpp\
	widgets/geWidget.cpp

GE_includedir = $(includedir)/GE
GE_include_HEADERS = ${GE_HEADS}
