
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
  window -title \"GameExporter\" -widthHeight 199 700 GWndMain; \
  \
  scrollLayout -childResizable true -width 200; \
  columnLayout -adjustableColumn true -cw 100 -cat \"both\" 0 -rs 0 GColMain; \
  \
  frameLayout -label \"Mesh\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true -vis false GFrmMesh; \
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
  frameLayout -label \"Static Mesh \\\\ Skin Pose\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  columnLayout -columnAlign \"left\" -adjustableColumn true -rs 2 GColPose; \
  checkBox -label \"Export with skin\" -onCommand \"GCmdSkinOn\" -offCommand \"GCmdSkinOff\" GChkSkin; \
  separator -height 10 -style \"none\"; \
  text -label \"File name:\"; \
  \
  rowLayout -nc 2 -adj 1 -cw 2 20 -cat 2 \"both\" 0; \
  textField GTxtFile; \
  button -label \"...\" -align \"center\" -command \"GCmdBrowseFile\"; \
  \
  setParent GColPose; \
  separator -height 10 -style \"none\"; \
  button -label \"Export\" -align \"center\" -command \"GCmdExport\"; \
  \
  setParent GColMain; \
  frameLayout -label \"New Animation\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  columnLayout -columnAlign \"left\" -adjustableColumn true -rs 2 GColNewAnim; \
  text -label \"Name:\"; \
  textField GTxtAnimName; \
  \
  rowLayout -nc 2 -adj 2 -cw 1 70 -cat 1 \"both\" 0; \
  text -label \"Start frame:\"; \
  textField GTxtStartFrame; \
  \
  setParent GColNewAnim; \
  rowLayout -nc 2 -adj 2 -cw 1 70 -cat 1 \"both\" 0; \
  text -label \"End frame:\"; \
  textField GTxtEndFrame; \
  \
  setParent GColNewAnim; \
  separator -height 10 -style \"none\"; \
  button -label \"Process\" -align \"center\" -command \"GCmdNewAnim\"; \
  \
  setParent GColMain; \
  frameLayout -label \"Character\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  columnLayout -columnAlign \"left\" -adjustableColumn true -rs 2 GColAnim; \
  text -label \"File name:\"; \
  \
  rowLayout -nc 2 -adj 1 -cw 2 20 -cat 2 \"both\" 0; \
  textField -enable false GTxtSkinFile; \
  button -label \"...\" -align \"center\" -command \"GCmdBrowseSkinFile\"; \
  \
  setParent GColAnim; \
  separator -height 10 -style \"none\"; \
  textScrollList -height 80 GLstAnim; \
  \
  rowLayout -nc 2 -adj 2 -cw 1 70 -ct2 \"both\" \"both\" -cl2 \"center\" \"center\"; \
  button -label \"Remove\" -align \"center\"; \
  button -label \"Add New\" -align \"center\"; \
  \
  setParent GColAnim; \
  separator -height 10 -style \"none\"; \
  button -label \"Save\" -align \"center\" -command \"GCmdSkinSave\"; \
  \
  setParent GColMain; \
  frameLayout -label \"Status\" -bv true -bs \"etchedIn\" -mh 10 -mw 10 -collapsable true; \
  text -label \"<Awaiting Command>\" GTxtStatus; \
  \
  showWindow GWndMain; \
  window -edit -width 200 GWndMain; \
  GCmdRestoreGui; \
} \
global proc GCmdClose() \
{ \
  trace \"Closing GExporter...\"; \
  if (`window -exists GWndMain`) \
  { deleteUI GWndMain; }; \
  if (`windowPref -exists GWndMain`) \
  { windowPref -remove GWndMain; }; \
} \
global proc GCmdCreateMenu() \
{ \
  trace \"Creating GExporter menu...\"; \
  global string $gMainWindow; \
	menu -parent $gMainWindow -label \"GExporter\" GMenuMain; \
	menuItem -label \"Open\" -command \"GCmdOpen\" GMenuOpen; \
} \
global proc GCmdDestroyMenu() \
{ \
  if (`menu -exists GMenuMain`) \
  { deleteUI GMenuMain; };\
} \
";

bool g_chkExportSkin = false;
CharString g_outFileName;
CharString g_skinFileName;
CharString g_statusText;
MaxCharacter *g_character = NULL;

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

void setTextLabel (const CharString &text, const CharString &label)
{
  CharString cmd = CharString("text -edit -label \"") + label + "\" " + text;
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
  setTextLabel( "GTxtStatus", g_statusText );
}

void clearStatus ()
{
  g_statusText = "";
  setTextLabel( "GTxtStatus", "" );
}

bool findNodeInSelection (MFn::Type type, MObject &pick)
{
  //Get the active selection
  bool pickFound = false;
  MSelectionList selList;
  MGlobal::getActiveSelectionList( selList );

  //Walk the selected nodes
  for (MItSelectionList selIt( selList ); !selIt.isDone(); selIt.next())
  {
    //Get the DAG path to the node
    MDagPath dagPath;
    selIt.getDagPath( dagPath );

    //Output the node path and api type
    CharString apiStr = dagPath.node().apiTypeStr();
    trace( CharString("findNodeInSelection: ")
           + dagPath.fullPathName().asChar()
           + " (" + apiStr + ")" );

    //Skip picking if found previously
    if (pickFound) continue;

    //Go down the tree at this selection
    MItDag dagSubIt; dagSubIt.reset(dagPath, MItDag::kBreadthFirst);
    for (; !dagSubIt.isDone(); dagSubIt.next())
    {
      MObject dagSubObj = dagSubIt.currentItem();
      MFnDagNode dagSubNode( dagSubObj );

      //Output the sub node path and api type
      CharString apiStr = dagSubObj.apiTypeStr();
      trace( CharString("findNodeInSelection: - ")
             + dagSubIt.fullPathName().asChar()
             + " (" + apiStr + ")" );

      //Skip intermediate objects and transform nodes
      if (dagSubNode.isIntermediateObject()) continue;
      if (dagSubObj.apiType() == MFn::kTransform) continue;

      //Check for the requested api type
      if (!dagSubObj.hasFn( type )) break;
      pick = dagSubIt.currentItem();
      pickFound = true;
      break;
    }
  }

  //Make sure we got something
  if (!pickFound) {
    trace( "findNodeInSelection: no selected node satisfies criteria!" );
    return false;
  }

  //Report
  MFnDagNode pickNode( pick );
  trace( "findNodeInSelection: picked '"
          + CharString( pickNode.name().asChar() )
          + "' (" + pick.apiTypeStr() + ")" );

  return true;
}

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

    MGlobal::executeCommand( "fileDialog -mode 1", result );
    if (result.length() == 0) return MStatus::kSuccess;

    g_outFileName = result.asChar();
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
    trace( "\\nExport: starting..." );

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
      SerializeManager sm;
      outFile.write( sm.getSignature(), sm.getSignatureSize() );
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
-----------------------------------------------
Command for browsing the character input file
-----------------------------------------------*/

class CmdBrowseSkinFile : public MPxCommand
{ public:
  static void* creator() { return new CmdBrowseSkinFile(); }
  virtual MStatus doIt (const MArgList &args)
  {
    MString result;
    clearStatus();
    setStatus( "Loading character..." );

    //Get the file from the browse dialog
    MGlobal::executeCommand( "fileDialog -mode 0", result );
    if (result.length() == 0) {
      setStatus( "No file selected" );
      return MStatus::kFailure; }

    //Open the file
    File file( result.asChar() );
    if (!file.open("rb")) {
      setStatus( "Failed opening file!" );
      return MStatus::kFailure; }

    //Check the file signature
    SerializeManager sm;
    if (file.getSize() < sm.getSignatureSize()) {
      file.close(); setStatus( "Invalid file!" );
      return MStatus::kFailure; }

    ByteString sig = file.read( sm.getSignatureSize() );
    if ((UintSize) sig.length() < sm.getSignatureSize()) {
      file.close(); setStatus( "Invalid file!" );
      return MStatus::kFailure; }

    if ( ! sm.checkSignature( sig.buffer() )) {
      file.close(); setStatus( "Invalid file!" );
      return MStatus::kFailure; }

    //Read the rest of the file
    ByteString data = file.read( file.getSize() - sm.getSignatureSize() );
    if (data.length() == 0) {
      file.close(); setStatus( "Invalid file!" );
      return MStatus::kFailure; }

    //Close the file
    file.close();

    //Load the character data
    ClassPtr newCls;
    MaxCharacter *newChar = (MaxCharacter*) sm.load( data.buffer(), &newCls );
    if (newCls != Class(MaxCharacter) || newChar == NULL) {
      setStatus( "Invalid file!" );
      return MStatus::kFailure; }

    //Free old data
    if (g_character != NULL)
      delete g_character;

    //Update GUI
    g_character = newChar;
    g_skinFileName = result.asChar();
    setTextFieldText( "GTxtSkinFile", g_skinFileName );

    setStatus( "Done." );
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
    if (g_chkExportSkin) {
      MGlobal::executeCommand( "checkBox -edit -value true GChkSkin" );
      //MGlobal::executeCommand( "frameLayout -edit -vis true GFrmSkin" );
    }

    if (g_outFileName.length() > 0)
      setTextFieldText( "GTxtFile", g_outFileName );

    if (g_skinFileName.length() > 0)
      setTextFieldText( "GTxtSkinFile", g_skinFileName );

    if (g_statusText.length() > 0)
      setTextLabel( "GTxtStatus", g_statusText );

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
  g_chkExportSkin = false;
  g_outFileName = "";
  g_statusText = "";
  g_character = NULL;

  //Init plugin
	MFnPlugin plugin( obj, "RedPill Studios", "0.1", "Any");
  plugin.registerCommand( "GCmdExport", CmdExport::creator );
  plugin.registerCommand( "GCmdRestoreGui", CmdRestoreGui::creator );
  plugin.registerCommand( "GCmdSkinOn", CmdSkinOn::creator );
  plugin.registerCommand( "GCmdSkinOff", CmdSkinOff::creator );
  plugin.registerCommand( "GCmdBrowseFile", CmdBrowseFile::creator );
  plugin.registerCommand( "GCmdBrowseSkinFile", CmdBrowseSkinFile::creator );

  //Define MEL commands
  MGlobal::executeCommand( mel );

  //Open GUI
  MGlobal::executeCommand( "GCmdCreateMenu" );
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

  //Cleanup
  if (g_character != NULL)
    delete g_character;
  
  //Close GUI
  MGlobal::executeCommand( "GCmdDestroyMenu" );
  MGlobal::executeCommand( "GCmdClose" );

  //Deinit plugin
  MFnPlugin plugin( obj );
  plugin.deregisterCommand( "GCmdExport" );
  plugin.deregisterCommand( "GCmdRestoreGui" );
  plugin.deregisterCommand( "GCmdSkinOn" );
  plugin.deregisterCommand( "GCmdSkinOff" );
  plugin.deregisterCommand( "GCmdBrowseFile" );
  plugin.deregisterCommand( "GCmdBrowseSkinFile" );

	return status;
}