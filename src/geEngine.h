#ifndef __GAMEENGINE_H
#define __GAMEENGINE_H

//General definitions
#include "geDefs.h"

//Class management
#define CLASS_DLL_ACTION GE_API_ENTRY
#include "geClass.h"
#include "geProperty.h"

//Miscelaneous
#include "geVectors.h"
#include "geMatrix.h"
#include "geObject.h"

//Resources
#include "geResource.h"
#include "geSerialize.h"
#include "geSkinPolyMesh_Res.h"
#include "geSkeleton_Res.h"
#include "geSkelAnim_Res.h"

//Actors
#include "geHmesh.h"
#include "geUVMesh.h"
#include "geDynamicMesh.h"
#include "geStaticMesh.h"
#include "geTexture.h"
#include "geShaders.h"
#include "geShader.h"
#include "geMaterial.h"
#include "geScene.h"
#include "geShape.h"
#include "geCamera.h"

//Widgets
#include "widgets/geWidget.h"
#include "widgets/geLabel.h"
#include "widgets/geFpsLabel.h"

//Loading & rendering
#include "geRenderer.h"
#include "geLoader.h"
#include "geLoad3ds.h"
#include "geLoadObj.h"
#include "geSaveObj.h"
#include "geShaders.h"
#include "geKernel.h"

//Prevent
#include "geUnClass.h"


#endif /* __GAMEENGINE_H */
