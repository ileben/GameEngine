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

#include <util/geUtil.h>
#include <engine/geEngine.h>
using namespace GE;

extern MObject g_meshNode;
extern bool g_gotMeshNode;
extern bool g_chkExportSkin;
extern CharString g_outFileName;
extern CharString g_statusText;

CharString operator+ (const char *cstr, const CharString &str);
void trace( const CharString &s);
void setStatus (const CharString &msg);
void clearStatus ();

void exportNoSkin (void **outData, UintSize *outSize);
void exportWithSkin (void **outData, UintSize *outSize);

#endif//__GEMAYA_H
