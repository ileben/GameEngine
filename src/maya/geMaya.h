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
#include <maya/MFnCamera.h>

#include <util/geUtil.h>
#include <engine/geEngine.h>
using namespace GE;

CharString operator+ (const char *cstr, const CharString &str);
void trace( const CharString &s);
void setStatus (const CharString &msg);
void clearStatus ();

Matrix4x4 exportMatrix (const MMatrix &m);
Vector3 exportPoint (const MFloatPoint &p);
Vector3 exportVector (const MVector &v);
Vector2 exportUV (double u, double v);
Quat exportQuat (const MQuaternion &q);
Matrix4x4 exportScale (double s[3]);
Vector3 exportColor (const MColor &c);

SkinAnim* exportAnimation (int start, int end, int fps);
Material* exportMaterial (const MObject &meshNode);
Matrix4x4 exportMatrix (const MMatrix &m);
Light* exportLight (const MDagPath &lightDagPath);
Camera* exportCamera (const MDagPath &camDagPath);

bool findNodeInSelection (MFn::Type type, MObject &pick);
bool findSkinForMesh (const MObject &meshNode, MObject &skinNode);
bool findSkinJointRoot (const MObject &skinNode, MObject &rootJoint);

void exportNoSkin (const MObject &meshNode, bool tangents, void **outData, UintSize *outSize);
void exportWithSkin (const MObject &meshNode, bool tangents, void **outData, UintSize *outSize);

#endif//__GEMAYA_H
