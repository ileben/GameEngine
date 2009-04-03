
#include "geMaya.h"
#include <maya/MFnPlugin.h> //Must only be included once!

/*
This MEL script links the C++ code and the GUI. */

const char *mel =  " \
\
global proc GCmdOpen() \
{ \
  if (`window -exists GWndMain`) return; \
  if (`windowPref -exists GWndMain`) \
  { windowPref -remove GWndMain; }; \
  \
  trace \"Opening GExporter...\"; \
  window -title \"GExporter\" -widthHeight 199 500 GWndMain; \
  \
  scrollLayout -childResizable true -width 200; \
  columnLayout -adjustableColumn true -cw 100 -cat \"both\" 0 -rs 0 GColMain; \
  \
  frameLayout -label \"Mesh\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  columnLayout -columnAlign \"left\" -adjustableColumn true -rs 2; \
  text -label \"Mesh node:\"; \
  button -label \"<Pick selected>\" -align \"center\" -command \"GCmdPickMesh\" GBtnPickMesh; \
  separator -height 10 -style \"none\"; \
  \
  setParent GColMain; \
  frameLayout -label \"Skin\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true -vis false GFrmSkin; \
  columnLayout -columnAlign \"left\" -adjustableColumn true -rs 2; \
  text -label \"Root joint:\"; \
  button -label \"<Pick selected>\" -align \"center\" -command \"GCmdPickJoint\" GBtnPickJoint; \
  \
  setParent GColMain; \
  frameLayout -label \"Output\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  columnLayout -columnAlign \"left\" -adjustableColumn true -rs 2 GColSkin; \
  checkBox -label \"Export with skin\" -onCommand \"GCmdSkinOn\" -offCommand \"GCmdSkinOff\" GChkSkin; \
  separator -height 10 -style \"none\"; \
  text -label \"File name:\"; \
  \
  rowLayout -nc 2 -adj 1 -cw 2 20 -cat 2 \"both\" 0; \
  textField GTxtFile; \
  button -label \"...\" -align \"center\" -command \"GCmdBrowseFile\"; \
  \
  setParent GColSkin; \
  separator -height 10 -style \"none\"; \
  button -label \"Export\" -align \"center\" -command \"GCmdExport\"; \
  \
  setParent GColMain; \
  frameLayout -label \"Status\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  text -label \"<Awaiting Command>\" GTxtStatus; \
  \
  showWindow GWndMain; \
  window -edit -widthHeight 200 500 GWndMain; \
  GCmdRestoreGui; \
} \
global proc GCmdClose() \
{ \
  trace \"Closing GExporter...\"; \
  if (`window -exists GWndMain`) \
  { deleteUI GWndMain; }; \
  if (`windowPref -exists GWndMain`) \
  { windowPref -remove GWndMain; }; \
  if (`menu -exists GMenuMain`) \
  { deleteUI GMenuMain; };\
} \
global proc GCmdCreateMenu() \
{ \
  trace \"Creating GExporter menu...\"; \
  global string $gMainWindow; \
	menu -parent $gMainWindow -label \"GExporter\" GMenuMain; \
	menuItem -label \"Open\" -command \"GCmdOpen\" GMenuOpen; \
} \
GCmdCreateMenu; \
";

MObject g_meshNode;
bool g_gotMeshNode = false;
bool g_chkExportSkin = false;
CharString g_outFileName;
CharString g_statusText;

/*
---------------------------------------
Helper functions
---------------------------------------*/

CharString operator+ (const char *cstr, const CharString &str)
{
  return CharString(cstr) + str;
}

void trace( const CharString &s)
{
  CharString ss = "trace \"" + s + "\"";
  MGlobal::executeCommand( ss.buffer() );
}

void setButtonLabel (const CharString &button, const CharString &label)
{
  CharString cmd = CharString("button -edit -label \"") + label + "\" " + button;
  MGlobal::executeCommand( cmd.buffer() );
}

void setTextFieldText (const CharString &textField, const CharString &text)
{
  CharString cmd = CharString("textField -edit -text \"") + text + "\" " + textField;
  MGlobal::executeCommand( cmd.buffer() );
}

CharString getTextFieldText (const CharString &textField)
{
  MString result;
  CharString cmd = "textField -query -text " + textField;
  MGlobal::executeCommand( cmd.buffer(), result );
  return result.asChar();
}

void setStatus (const CharString &msg)
{
  g_statusText += msg + "\\n";
  CharString cmd = CharString("text -edit -label \"") + g_statusText + "\" GTxtStatus";
  MGlobal::executeCommand( cmd.buffer() );
}

void clearStatus ()
{
  g_statusText = "";
  MGlobal::executeCommand( "text -edit -label \"\" GTxtStatus" );
}

/*
----------------------------------------------------------------
Command that picks a node from the current selection matching
the given criteria and sets the label of a button to the
node's name.
--------------------------------------------------------------*/

class CmdPickToButton : public MPxCommand
{
protected:
  CharString buttonName;
  MFn::Type nodeType;
  MObject pickObj;

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
    MSelectionList selList;
    MGlobal::getActiveSelectionList( selList );
    for (MItSelectionList selIt( selList ); !selIt.isDone(); selIt.next())
    {
      MDagPath dagPath;
      selIt.getDagPath( dagPath );

      //Output the node path and api type
      CharString apiStr = dagPath.node().apiTypeStr();
      trace( CharString("- ") + dagPath.fullPathName().asChar() + " (" + apiStr + ")" );
      if (pickFound) continue;

      //Go down the tree at this selection
      MItDag dagSubIt; dagSubIt.reset(dagPath, MItDag::kBreadthFirst);
      for (; !dagSubIt.isDone(); dagSubIt.next())
      {
        MObject dagSubObj = dagSubIt.currentItem();
        MFnDagNode dagSubNode( dagSubObj );

        //Skip intermediate objects and transform nodes
        if (dagSubNode.isIntermediateObject()) continue;
        if (dagSubObj.apiType() == MFn::kTransform) continue;

        //Check for the requested api type
        if (!dagSubObj.hasFn( nodeType )) break;
        pickObj = dagSubIt.currentItem();
        pickFound = true;
        break;
      }
    }

    //Make sure we got something
    if (!pickFound) {
      trace( "Pick: No selected node satisfies criteria!" );
      return MStatus::kFailure;
    }
    
    //Change the title of the button to the name of the node
    MFnDagNode pickNode( pickObj );
    setButtonLabel( buttonName, pickNode.name().asChar() );

    //Report the pick
    trace( "Pick: picked " + CharString(pickNode.fullPathName().asChar()) );
    return MStatus::kSuccess;
  }
};

/*
---------------------------------------
Command for the PickMesh button
---------------------------------------*/

class CmdPickMesh : public CmdPickToButton
{ public:
  CmdPickMesh() : CmdPickToButton("GBtnPickMesh", MFn::kMesh) {}
  static void* creator() { return new CmdPickMesh; }
  virtual MStatus doIt (const MArgList &args)
  {
    //Pick a node
    if (CmdPickToButton::doIt( args ) == MStatus::kFailure)
      return MStatus::kFailure;

    //Store the picked node
    g_meshNode = pickObj;
    g_gotMeshNode = true;
    return MStatus::kSuccess;
  }
};

/*
------------------------------------------
Command for the "Export skin" checkbox
------------------------------------------*/

class CmdSkinOn : public MPxCommand
{ public:
  static void *creator() { return new CmdSkinOn; }
  virtual MStatus doIt (const MArgList &args)
  {
    g_chkExportSkin = true;
    //MGlobal::executeCommand( "frameLayout -edit -vis true GFrmSkin" );
    return MStatus::kSuccess;
  }
};

class CmdSkinOff : public MPxCommand
{ public:
  static void *creator() { return new CmdSkinOff; }
  virtual MStatus doIt (const MArgList &args)
  {
    g_chkExportSkin = false;
    //MGlobal::executeCommand( "frameLayout -edit -vis false GFrmSkin" );
    return MStatus::kSuccess;
  }
};

/*
--------------------------------------------
Command for browsing the output file
--------------------------------------------*/

class CmdBrowseFile : public MPxCommand
{ public:
  static void* creator() { return new CmdBrowseFile(); }
  virtual MStatus doIt (const MArgList &args)
  {
    MString result;
    int a = 0;
    MGlobal::executeCommand( "fileDialog -mode 1", result );
    if (result.length() == 0) return MStatus::kSuccess;

    g_outFileName = result.asChar();
    setTextFieldText( "GTxtFile", g_outFileName );

    return MStatus::kSuccess;
  }
};

/*
------------------------------------------------------
Command that sets the GUI up in the previous state
after is has been reopened.
-----------------------------------------------------*/

class CmdRestoreGui : public MPxCommand
{ public:
  static void* creator() { return new CmdRestoreGui; }
  virtual MStatus doIt (const MArgList &args)
  {
    if (g_gotMeshNode) {
      MFnDagNode node( g_meshNode );
      setButtonLabel( "GBtnPickMesh", node.name().asChar() );
    }

    if (g_chkExportSkin) {
      MGlobal::executeCommand( "checkBox -edit -value true GChkSkin" );
      //MGlobal::executeCommand( "frameLayout -edit -vis true GFrmSkin" );
    }

    if (g_outFileName.length() > 0)
      setTextFieldText( "GTxtFile", g_outFileName );

    return MStatus::kSuccess;
  }
};

/*
---------------------------------------
Command for the Export button
---------------------------------------*/

class CmdExport : public MPxCommand
{
public:
  static void* creator () { return new CmdExport(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Exporting..." );
    trace( "Export: starting..." );

    void *outData = NULL;
    UintSize outSize = 0;

    /*
    //Make sure an output file was picked
    CharString outFileName = getTextFieldText( "GTxtFile" );
    if (outFileName.length() == 0) {
      setStatus( "Output file missing!\\nAborted." );
      return MStatus::kFailure;
    }
    */

    //Make sure mesh node was picked
    if (!g_gotMeshNode) {
      setStatus( "Mesh node missing!\\nAborted." );
      trace( "Export: mesh node missing!" );
      return MStatus::kFailure;
    }

    if (g_chkExportSkin)
    {
      //Export skin pose
      exportWithSkin( &outData, &outSize );
    }
    else
    {
      //Export static mesh
      exportNoSkin( &outData, &outSize );
    }

    //Make sure something was actually exported
    if (outSize == 0) {
      setStatus( "Zero data size!\\nAborted." );
      trace( "Export: received zero data!" );
      return MStatus::kFailure;
    }

    //Write to file
    File outFile( "C:\\Projects\\Programming\\GameEngine\\test\\mayatest.pak" );
    //File outFile( outFileName );
    if (outFile.open( "wb" )) {
      outFile.write( outData, (int)outSize );
      outFile.close();
    }else {
      setStatus( "Failed writing to file." );
      trace( "Export: failed opening output file for writing!" );
    }

    setStatus( "Done." );
    trace( "Export: done" );
    return MStatus::kSuccess;
  }
};

/*
--------------------------------------------------------------
initializePlugin is called by Maya when the plugin is loaded.
--------------------------------------------------------------*/

MStatus initializePlugin ( MObject obj )
{ 
  MStatus status = MStatus::kSuccess;

  //Reset global data
  g_gotMeshNode = false;
  g_chkExportSkin = false;
  g_outFileName = "";
  g_statusText = "";

  //Init plugin
	MFnPlugin plugin( obj, "RedPill Studios", "0.1", "Any");
  plugin.registerCommand( "GCmdExport", CmdExport::creator );
  plugin.registerCommand( "GCmdPickMesh", CmdPickMesh::creator );
  plugin.registerCommand( "GCmdRestoreGui", CmdRestoreGui::creator );
  plugin.registerCommand( "GCmdSkinOn", CmdSkinOn::creator );
  plugin.registerCommand( "GCmdSkinOff", CmdSkinOff::creator );
  plugin.registerCommand( "GCmdBrowseFile", CmdBrowseFile::creator );

  //Define MEL commands
  MGlobal::executeCommand( mel );

  //Open GUI
  MGlobal::executeCommand( "GCmdOpen" );

	return status;
}

/*
-------------------------------------------------------------------
uninitializePlugin is called by Maya when the plugin is unloaded.
-------------------------------------------------------------------*/

MStatus uninitializePlugin( MObject obj)
{
  MStatus status = MStatus::kSuccess;
  
  //Close GUI
  MGlobal::executeCommand( "GCmdClose" );

  //Deinit plugin
  MFnPlugin plugin( obj );
  plugin.deregisterCommand( "GCmdExport" );
  plugin.deregisterCommand( "GCmdPickMesh" );
  plugin.deregisterCommand( "GCmdRestoreGui" );
  plugin.deregisterCommand( "GCmdSkinOn" );
  plugin.deregisterCommand( "GCmdSkinOff" );
  plugin.deregisterCommand( "GCmdBrowseFile" );

	return status;
}