#ifndef __GAMEENGINE_H
#define __GAMEENGINE_H

//#define CLASS_DLL_ACTION GE_API_ENTRY
#include "util/geUtil.h"
#include "io/geFile.h"
#include "image/geImage.h"
#include "math/geMath.h"

//Resources
#include "geProperty.h"
#include "geResource.h"
#include "geSkinMesh.h"
#include "geSkinPose.h"
#include "geSkinAnim.h"
#include "geCharacter.h"

#include "geTexture.h"
#include "geShaders.h"
#include "geShader.h"
#include "geMaterial.h"
#include "geHmesh.h"
#include "gePolyMesh.h"
#include "geTexMesh.h"
#include "geTriMesh.h"
#include "gePrimitives.h"

//Actors
#include "geActor.h"
#include "geScene.h"
#include "geCamera.h"
#include "geLight.h"
#include "actors/geTriMeshActor.h"
#include "actors/gePolyMeshActor.h"
#include "actors/geSkinMeshActor.h"
#include "actors/geAxisActor.h"

//Widgets
#include "widgets/geWidget.h"
#include "widgets/geLabel.h"
#include "widgets/geFpsLabel.h"

//Loading & rendering
#include "geRenderer.h"
#include "geShaders.h"
#include "geKernel.h"

//Controllers
#include "geController.h"

//Prevent
//#include "geUnClass.h"


#endif /* __GAMEENGINE_H */
