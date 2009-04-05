#ifndef __GEMAYA_H
#define __GEMAYA_H

#include <iostream> //Maya needs this
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFloatArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MAnimControl.h>

#include <util/geUtil.h>
#include <engine/geEngine.h>
using namespace GE;

CharString operator+ (const char *cstr, const CharString &str);
void trace( const CharString &s);
void setStatus (const CharString &msg);
void clearStatus ();

bool findNodeInSelection (MFn::Type type, MObject &pick);
void exportNoSkin (void **outData, UintSize *outSize);
void exportWithSkin (void **outData, UintSize *outSize);
SkinAnim* exportAnimation (int start, int end, int fps);

#endif//__GEMAYA_H
