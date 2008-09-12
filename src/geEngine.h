#ifndef __GAMEENGINE_H
#define __GAMEENGINE_H

//General definitions
#include "geDefs.h"

//External headers
#include <list>
#include <assert.h>
#include <OpenCC/OpenCC.h>

//General definitions
#include "geDefs.h"

//Add DLL interface to class declarations
#define CLASS_DLL_ACTION GE_API_ENTRY

//Class management
#include "geClass.h"
#include "geProperty.h"

//API headers - scene tree
#include "geVectors.h"
#include "geMatrix.h"
#include "geObject.h"
#include "geResource.h"
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

//API headers - widgets
#include "widgets/geWidget.h"
#include "widgets/geLabel.h"
#include "widgets/geFpsLabel.h"

//API headers - loading & rendering
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
