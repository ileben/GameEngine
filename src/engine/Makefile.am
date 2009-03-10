lib_LTLIBRARIES = libGameEngine.la

GE_HEADS =\
	geActor.h\
	geAnimation.h\
	geCamera.h\
	geCharacter.h\
	geEngine.h\
	geGLHeaders.h\
	geHmesh.h\
	geHmeshAdjiter.h\
	geHmeshDataiter.h\
	geKernel.h\
	geLight.h\
	geLoad3ds.h\
	geLoadObj.h\
	geLoader.h\
	geMaterial.h\
	geMatrix.h\
	geObject.h\
	gePolyMesh.h\
	gePolyMeshIters.h\
	gePrimitives.h\
	geProperty.h\
	geRenderer.h\
	geResource.h\
	geSaveObj.h\
	geScene.h\
	geShader.h\
	geShaders.h\
	geSkinAnim.h\
	geSkinMesh.h\
	geSkinPose.h\
	geTexture.h\
	geTexMesh.h\
	geTriMesh.h\
	geVectors.h\
	actors/geAxisActor.h\
	actors/gePolyMeshActor.h\
	actors/geTriMeshActor.h\
	actors/geSkinMeshActor.h\
	widgets/geFpsLabel.h\
	widgets/geLabel.h\
	widgets/geWidget.h

libGameEngine_la_CXXFLAGS = -I${prefix}/include -I$(top_builddir)/src
libGameEngine_la_LDFLAGS = -L${prefix}/lib
libGameEngine_la_SOURCES =\
	${GE_HEADS}\
	geActor.cpp\
	geCamera.cpp\
	geCharacter.cpp\
	geHMesh.cpp\
	geKernel.cpp\
	geLight.cpp\
	geLoad3ds.cpp\
	geLoadObj.cpp\
	geLoadTex.cpp\
	geLoader.cpp\
	geMaterial.cpp\
	geMatrix.cpp\
	geObject.cpp\
	gePolyMesh.cpp\
	gePrimitives.cpp\
	geProperty.cpp\
	geRenderer.cpp\
	geResource.cpp\
	geSaveObj.cpp\
	geScene.cpp\
	geShaders.cpp\
	geShader.cpp\
	geSkinAnim.cpp\
	geSkinMesh.cpp\
	geSkinPose.cpp\
	geTexture.cpp\
	geTexMesh.cpp\
	geTriMesh.cpp\
	geVectors.cpp\
	actors/geAxisActor.cpp\
	actors/gePolyMeshActor.cpp\
	actors/geTriMeshActor.cpp\
	actors/geSkinMeshActor.cpp\
	widgets/geFpsLabel.cpp\
	widgets/geLabel.cpp\
	widgets/geWidget.cpp

GE_includedir = $(includedir)/GameEngine/engine
GE_include_HEADERS = ${GE_HEADS}
