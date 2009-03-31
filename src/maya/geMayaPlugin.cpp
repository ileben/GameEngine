
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxCommand.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSkinCluster.h>

#include <util/geUtil.h>
using namespace GE;

/*
This MEL script links the C++ code and the GUI. */

const char *mel =  " \
global proc GExporterOpen() \
{ \
  trace \"Opening GExporter...\"; \
  window -title \"GExporter\" -widthHeight 200 400 GWndMain; \
  columnLayout -adjustableColumn true -columnWidth 100 -rowSpacing 10 \
    -columnOffset \"both\" 25; \
  button -label \"Export\" -command \"GCmdExport\"; \
  button -label \"<Pick Mesh>\" -command \"GCmdPickMesh\" GBtnPickMesh; \
  button -label \"<Pick Joint>\" -command \"GCmdPickJoint\" GBtnPickJoint; \
  showWindow GWndMain; \
} \
global proc GExporterClose() \
{ \
  trace \"Closing GExporter...\"; \
  if (`window -exists GWndMain`) \
  { deleteUI GWndMain; } \
  if (`windowPref -exists GWndMain`) \
  { windowPref -remove GWndMain; } \
  deleteUI menuGExporter; \
} \
global proc GExporterCreateMenu() \
{ \
  trace \"Creating GExporter menu...\"; \
  global string $gMainWindow; \
	menu -parent $gMainWindow -label \"GExporter\" menuGExporter; \
	menuItem -label \"Open\" -command \"GExporterOpen\" menuGExporterOpen; \
} \
GExporterCreateMenu; \
GExporterOpen; \
";

static MFnDagNode meshNode;
static MFnDagNode jointNode;
static bool gotMeshNode = false;
static bool gotJointNode = false;


CharString operator+ (const char *cstr, const CharString &str)
{
  return CharString(cstr) + str;
}

void trace( const CharString &s)
{
  CharString ss = "trace \"" + s + "\"";
  MGlobal::executeCommand( ss.buffer() );
}

class CmdPickToButton : public MPxCommand
{
private:
  CharString buttonName;
  MFn::Type nodeType;

public:
  CmdPickToButton (const CharString &button, MFn::Type type)
  {
    buttonName = button;
    nodeType = type;
  };

  virtual MStatus doIt (const MArgList &args)
  {
    MStatus status; 

    //Find first kMesh among selection
    trace( "Pick: current selection:" );
    bool pickFound = false;
    MFnDagNode pickNode;
    MSelectionList selList;
    MGlobal::getActiveSelectionList( selList );
    for (MItSelectionList selIt( selList ); !selIt.isDone(); selIt.next())
    {
      MDagPath dagPath;
      selIt.getDagPath( dagPath );

      //Output the node path and api type
      CharString apiStr = dagPath.node().apiTypeStr();
      trace( CharString("- ") + dagPath.fullPathName().asChar() + " (" + apiStr + ")" );

      //Check if it's got the requested api
      if (dagPath.hasFn( nodeType ) && !pickFound) {
        pickNode.setObject( dagPath );
        pickFound = true;
      }
    }

    //Make sure we got something
    if (!pickFound) {
      trace( "Pick: No selected node satisfies criteria!" );
      return MStatus::kFailure;
    }
    
    //Change the title of the button to the name of the node
    CharString cmd = CharString("button -edit -label \"") + pickNode.name().asChar() + "\" " + buttonName;
    MGlobal::executeCommand( cmd.buffer() );

    //Report the pick
    trace( "Pick: picked " + CharString(pickNode.fullPathName().asChar()) );
    return MStatus::kSuccess;
  }
};

class CmdPickMesh : public CmdPickToButton
{ public:
  static void* creator() { return new CmdPickToButton("GBtnPickMesh", MFn::kMesh); }
};

class CmdPickJoint : public MPxCommand
{ public:
  static void *creator() { return new CmdPickToButton("GBtnPickJoint", MFn::kJoint); }
};

class CmdExport : public MPxCommand
{
public:
  static void* creator () { return new CmdExport(); }
  virtual MStatus doIt (const MArgList &args)
  {
    MItDag dagIt( MItDag::kDepthFirst, MFn::kInvalid );
    for (; !dagIt.isDone(); dagIt.next())
    {
      MStatus status;
      
      MDagPath dagPath;
      status = dagIt.getPath( dagPath );
      if (!status) continue;
      
      MFnDagNode dagNode( dagPath, &status );
      if (dagNode.isIntermediateObject()) continue;
      if (!dagPath.hasFn( MFn::kMesh )) continue;

      MFnMesh mesh( dagPath );
      trace( dagPath.fullPathName().asChar() );
    }

    return MStatus::kSuccess;
  }
};

/*
initializePlugin is called by Maya when the plugin is loaded. */

MStatus initializePlugin ( MObject obj )
{ 
  MStatus status = MStatus::kSuccess;

	MFnPlugin plugin( obj, "RedPill Studios", "0.1", "Any");
  plugin.registerCommand( "GCmdExport", CmdExport::creator );
  plugin.registerCommand( "GCmdPickMesh", CmdPickMesh::creator );
  plugin.registerCommand( "GCmdPickJoint", CmdPickJoint::creator );

  MGlobal::executeCommand( mel );
	return status;
}

/*
uninitializePlugin is called by Maya when the plugin is unloaded. */

MStatus uninitializePlugin( MObject obj)
{
  MStatus status = MStatus::kSuccess;
  MGlobal::executeCommand( "GExporterClose" );

  MFnPlugin plugin( obj );
  plugin.deregisterCommand( "GCmdExport" );
  plugin.deregisterCommand( "GCmdPickMesh" );
  plugin.deregisterCommand( "GCmdPickJoint" );

	return status;
}