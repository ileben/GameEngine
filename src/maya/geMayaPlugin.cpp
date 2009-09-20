
#include "geMaya.h"
#include <maya/MFnPlugin.h> //Must only be included once!
#include <maya/MBoundingBox.h>

//This MEL script links the C++ code and the GUI.
#include "mel.embedded"

bool g_chkExportSkin = false;
bool g_chkExportTangents = false;
CharString g_outFileName;
CharString g_skinFileName;
CharString g_statusText;
MaxCharacter *g_character = NULL;
SkinAnim *g_animation = NULL;

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

void enableControl (const CharString &controlType, const CharString &control)
{
  CharString cmd = controlType + " -edit -enable true " + control;
  MGlobal::executeCommand( cmd.buffer() );
}

void disableControl (const CharString &controlType, const CharString &control)
{
  CharString cmd = controlType + " -edit -enable false " + control;
  MGlobal::executeCommand( cmd.buffer() );
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

void appendListItem (const CharString &list, const CharString &item)
{
  CharString cmd = "textScrollList -edit -append \"" + item + "\" " + list;
  MGlobal::executeCommand( cmd.buffer() );
}

void removeListItem (const CharString &list, int index)
{
  CharString cmd = "textScrollList -edit -removeIndexedItem "
    + CharString::FInt( index + 1 ) + " " + list;
  MGlobal::executeCommand( cmd.buffer() );
}

int getListSelection (const CharString &list)
{
  MIntArray result;
  CharString cmd = "textScrollList -query -selectIndexedItem " + list;
  MGlobal::executeCommand( cmd.buffer(), result );
  
  if (result.length() > 0)
    return result[0] - 1;
  else return -1; 
}

void clearList (const CharString &list)
{
  CharString cmd = "textScrollList -edit -removeAll " + list;
  MGlobal::executeCommand( cmd.buffer() );
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
------------------------------------------
Command for the "Export tangets" checkbox
------------------------------------------*/

class CmdTangentsOn : public MPxCommand
{ public:
  static void *creator() { return new CmdTangentsOn; }
  virtual MStatus doIt (const MArgList &args)
  {
    g_chkExportTangents = true;
    return MStatus::kSuccess;
  }
};

class CmdTangentsOff : public MPxCommand
{ public:
  static void *creator() { return new CmdTangentsOff; }
  virtual MStatus doIt (const MArgList &args)
  {
    g_chkExportTangents = false;
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

    //Make sure an output file was picked
    CharString outFileName = getTextFieldText( "GTxtFile" );
    if (outFileName.length() == 0) {
      setStatus( "Output file missing!\\nAborted." );
      return MStatus::kFailure;
    }

    //Make sure a mesh node was selected
    MObject meshNode;
    if (!findNodeInSelection( MFn::kMesh, meshNode )) {
      setStatus( "Please select a mesh node!" );
      return MStatus::kFailure;
    }
    
    //Export skin pose or static mesh
    if (g_chkExportSkin)
      exportWithSkin( meshNode, g_chkExportTangents, &outData, &outSize);
    else exportNoSkin( meshNode, g_chkExportTangents, &outData, &outSize );

    //Make sure something was actually exported
    if (outSize == 0) {
      setStatus( "Zero data exported!\\nAborted." );
      trace( "Export: received zero data!" );
      return MStatus::kFailure;
    }

    //Open output file for writing
    File outFile( outFileName );
    if (!outFile.open( FileAccess::Write, FileCondition::Truncate )) {
      setStatus( "Failed writing to file." );
      trace( "Export: failed opening output file for writing!" );
      return MStatus::kFailure;
    }

    //Write to file
    SerializeManager sm;
    outFile.write( sm.getSignature(), sm.getSignatureSize() );
    outFile.write( outData, (int)outSize );
    outFile.close();

    setStatus( "Done." );
    trace( "Export: done" );
    return MStatus::kSuccess;
  }
};

/*
---------------------------------------
Command for the ExportAll button
---------------------------------------*/

class CmdExportAll : public MPxCommand
{
public:
  static void* creator () { return new CmdExportAll(); }

  bool writePackageFile (void *data, UintSize size, File &file)
  {
    //Create missing directories
    if (!file.createPath())
      return false;

    //Open output file for writing
    if (!file.open( FileAccess::Write, FileCondition::Truncate )) {
      trace( "Export: failed opening output file for writing!" );
      return false;
    }
    
    //Write data
    SerializeManager sm;
    file.write( sm.getSignature(), sm.getSignatureSize() );
    file.write( data, (int)size );
    file.close();
    return true;
  }

  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Exporting..." );
    trace( "\\nExport: starting..." );

    //Make sure an output file was picked
    CharString outFileName = getTextFieldText( "GTxtFile" );
    if (outFileName.length() == 0) {
      setStatus( "Output file missing!\\nAborted." );
      return MStatus::kFailure;
    }
    File outFile( outFileName );

    //Create root actor
    Actor3D *root = new Actor3D;
    Float worldScale = 1.0;
    int meshID = 0;

    //Walk the scene graph, searching for the scale ref dummy
    MItDag dagScaleIt( MItDag::kBreadthFirst );
    for ( ; !dagScaleIt.isDone(); dagScaleIt.next() )
    {
      //Get path to the node
      MDagPath dagPath;
      dagScaleIt.getPath( dagPath );

      //Check the name
      MFnDagNode dagNode( dagPath );
      CharString name = dagNode.name().asChar();
      if (name == "ScaleReference")
      {
        //Find the height of its bounding box
        MMatrix mworld = dagPath.inclusiveMatrix();
        MBoundingBox box = dagNode.boundingBox();
        box.transformUsing( mworld );
        Float h = (Float) box.height();

        //Adjust the exporting scale so that it matches 180 units
        worldScale = 180.0f / h;
        break;
      }
    }

    //Walk the whole scene graph
    MItDag dagIt( MItDag::kBreadthFirst );
    for ( ; !dagIt.isDone(); dagIt.next())
    {
      //Get path to the node
      MDagPath dagPath;
      dagIt.getPath( dagPath );

      //Skip intermediate objects
      MFnDagNode dagNode( dagPath );
      if (dagNode.isIntermediateObject()) continue;
      if (dagPath.hasFn( MFn::kTransform )) continue;

      //Get the name of the node and skip scale reference dummy
      //(A better method would be to have the dummy hidden and ignore hidden
      //objects but haven't figured out how to find out if one is hidden yet)
      CharString name = dagNode.name().asChar();
      if (name == "ScaleReference") {
        dagIt.prune();
        continue;
      }

      //Check if mesh found
      if (dagPath.hasFn( MFn::kMesh ))
      {
        //Add extension and remove the namespace prefix
        CharString meshName = name + ".pak";
        int colon = meshName.findRev( ":" );
        if (colon != -1) meshName = meshName.sub( colon+1 );

        //Only export mesh if not a file reference
        if (! dagNode.isFromReferencedFile())
        {
          //Export mesh data
          void *outData = NULL;
          UintSize outSize = 0;
          exportNoSkin( dagPath.node(), true, &outData, &outSize );

          //Write to file
          CharString meshName = name + ".pak";
          File meshFile = outFile.getRelativeFile( "Meshes/" + meshName );
          if (!writePackageFile( outData, outSize, meshFile )) {
            std::free( outData );
            continue;
          }
    
          //Free mesh data
          std::free( outData );
        }

        //Export material
        Material *material = exportMaterial( dagPath.node() );
        if (material == NULL) continue;

        //Get transform matrix
        Matrix4x4 matrix = exportMatrix( dagPath.inclusiveMatrix() );

        //Create a new actor for this mesh
        TriMeshActor *actor = new TriMeshActor;
        actor->setMesh( meshName );
        actor->setMatrix( matrix );
        actor->setMaterial( material );
        actor->setParent( root );
        actor->scale( worldScale );
      }
      else if (dagPath.hasFn( MFn::kLight ))
      {
        //Export light
        Light *light = exportLight( dagPath );
        if (light == NULL) continue;
        light->setParent( root );
        light->scale( worldScale );
      }
      else if (dagPath.hasFn( MFn::kCamera ))
      {
        //Skip "startup cameras" (front, side, top, persp)
        int isStartup = 0;
        CharString pname = dagPath.fullPathName().asChar();
        CharString cmd = "camera -query -startupCamera " + pname;
        MGlobal::executeCommand( cmd.buffer(), isStartup );
        if (isStartup != 0) continue;

        //Export camera
        Camera *cam = exportCamera( dagPath );
        if (cam == NULL) continue;
        cam->setParent( root );
        cam->scale( worldScale );
      }

      /*
      //Output the node path and api type
      CharString apiStr = dagPath.node().apiTypeStr();
      trace( CharString("findNodeInSelection: ")
             + dagPath.fullPathName().asChar()
             + " (" + apiStr + ")" );
      */
    }

    //Export actor data
    void *outData = NULL;
    UintSize outSize = 0;
    SerializeManager sm;
    sm.save( root, &outData, &outSize );

    //Write to file
    writePackageFile( outData, outSize, outFile );
    std::free( outData );

    setStatus( "Done." );
    trace( "Export: done" );
    return MStatus::kSuccess;
  }
};

void enableAnimProcess()
{
  disableControl( "button", "GBtnClearAnim" );
  enableControl( "textField", "GTxtAnimName" );
  enableControl( "textField", "GTxtAnimStart" );
  enableControl( "textField", "GTxtAnimEnd" );
  enableControl( "button", "GBtnProcessAnim" );
  setTextFieldText( "GTxtAnimName", "" );
  setTextFieldText( "GTxtAnimStart", "" );
  setTextFieldText( "GTxtAnimEnd", "" );
}

void disableAnimProcess()
{
  disableControl( "textField", "GTxtAnimName" );
  disableControl( "textField", "GTxtAnimStart" );
  disableControl( "textField", "GTxtAnimEnd" );
  disableControl( "button", "GBtnProcessAnim" );
  enableControl( "button", "GBtnClearAnim" );
}

class CmdProcessAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdProcessAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Processing animation..." );
    trace( "\\nAnimation: starting..." );

    //Make sure we have the animation name
    CharString animName = getTextFieldText( "GTxtAnimName" );
    if (animName.length() == 0) {
      setStatus( "Please enter animation name!" );
      return MStatus::kFailure;
    }

    //Make sure we have the start frame
    CharString animStartStr = getTextFieldText( "GTxtAnimStart" );
    if (animStartStr.length() == 0) {
      setStatus( "Please enter start frame!" );
      return MStatus::kFailure;
    }

    //Make sure we have the end frame
    CharString animEndStr = getTextFieldText( "GTxtAnimEnd" );
    if (animEndStr.length() == 0) {
      setStatus( "Please enter end frame!" );
      return MStatus::kFailure;
    }

    //Parse start and end values
    int startLen = 0, endLen;
    int animStart = animStartStr.parseIntegerAt( 0, &startLen );
    int animEnd = animEndStr.parseIntegerAt( 0, &endLen );
    if (startLen == 0 || endLen == 0) {
      setStatus( "Invalid start/end value!" );
      return MStatus::kFailure;
    }

    //Create new animation
    SkinAnim *newAnim = exportAnimation( animStart, animEnd, 24 );
    if (newAnim == NULL) {
      setStatus( "Aborted." );
      return MStatus::kFailure;
    }

    //Swap old data with new
    if (g_animation != NULL)
      delete g_animation;

    g_animation = newAnim;
    g_animation->name = animName;
    
    //Update GUI
    disableAnimProcess();

    setStatus( "Done." );
    trace( "Animation: done. " );
    return MStatus::kSuccess;
  }
};

class CmdClearAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdClearAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Deleting animation..." );

    //Free animation data
    if (g_animation != NULL)
      delete g_animation;

    g_animation = NULL;

    //Update GUI
    enableAnimProcess();

    setStatus( "Done." );
    return MStatus::kSuccess;
  };
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
    if (!file.open( FileAccess::Read, FileCondition::MustExist )) {
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

    //Swap old data with new
    if (g_character != NULL)
      delete g_character;

    g_skinFileName = result.asChar();
    g_character = newChar;

    //Update GUI
    setTextFieldText( "GTxtSkinFile", g_skinFileName );

    clearList( "GLstAnim" );
    for (UintSize a=0; a<g_character->anims.size(); ++a)
      appendListItem( "GLstAnim", g_character->anims[ a ]->name );

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdAddAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdAddAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Adding animation..." );

    //Make sure a character has been loaded
    if (g_character == NULL) {
      setStatus( "No character loaded!" );
      return MStatus::kFailure;
    }

    //Make sure an animation has been processed
    if (g_animation == NULL) {
      setStatus( "No animation processed!" );
      return MStatus::kFailure;
    }

    //Check if the number of joints match
    if (g_animation->tracksR.size() !=
        g_character->pose->joints.size()) {
      setStatus( "Joints don't match!" );
      return MStatus::kFailure;
    }

    //Update data
    g_character->anims.pushBack( g_animation );

    //Update GUI
    appendListItem( "GLstAnim", g_animation->name );
    
    //Clear animation data and GUI
    g_animation = NULL;
    enableAnimProcess();
    
    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdRemoveAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdRemoveAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Removing animation..." );

    //Make sure a character has been loaded
    if (g_character == NULL) {
      setStatus( "No character loaded!" );
      return MStatus::kFailure;
    }

    //Make sure there is any animations at all
    if (g_character->anims.size() == 0) {
      setStatus( "No animations present!" );
      return MStatus::kFailure;
    }

    //Get selection index
    int index = getListSelection( "GLstAnim" );
    if (index < 0 || index >= (int)g_character->anims.size()) {
      setStatus( "Invalid selection!" );
      return MStatus::kFailure;
    }

    //Update data
    g_character->anims.removeAt( index );

    //Update GUI
    removeListItem( "GLstAnim", index );
    
    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdSaveChar : public MPxCommand
{ public:
  static void* creator() { return new CmdSaveChar(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Saving character..." );

    //Make sure a character has been laoded
    if (g_character == NULL || g_skinFileName.length() == 0) {
      setStatus( "No character loaded!" );
      return MStatus::kFailure;
    }

    //Export character
    void *outData; UintSize outSize=0;
    SerializeManager sm;
    sm.save( g_character, &outData, &outSize );
    
    //Make sure something was actually exported
    if (outSize == 0) {
      setStatus( "Zero data exported!" );
      return MStatus::kFailure;
    }

    //Open output file for writing
    File outFile( g_skinFileName );
    if (!outFile.open( FileAccess::Write, FileCondition::Truncate )) {
      setStatus( "Failed writing to file." );
      trace( "Export: failed opening output file for writing!" );
      return MStatus::kFailure;
    }

    //Write to file
    outFile.write( sm.getSignature(), sm.getSignatureSize() );
    outFile.write( outData, (int)outSize );
    outFile.close();

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
    if (g_chkExportSkin)
      MGlobal::executeCommand( "checkBox -edit -value true GChkSkin" );

    if (g_chkExportTangents)
      MGlobal::executeCommand( "checkBox -edit -value true GChkTangents" );

    if (g_outFileName.length() > 0)
      setTextFieldText( "GTxtFile", g_outFileName );

    if (g_animation != NULL)
    {
      disableAnimProcess();
      setTextFieldText( "GTxtAnimName", g_animation->name);
    }

    if (g_skinFileName.length() > 0)
      setTextFieldText( "GTxtSkinFile", g_skinFileName );

    if (g_character != NULL)
    {
      for (UintSize a=0; a<g_character->anims.size(); ++a)
        appendListItem( "GLstAnim", g_character->anims[ a ]->name );
    }

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
  g_animation = NULL;

  //Init plugin
	MFnPlugin plugin( obj, "RedPill Studios", "0.1", "Any");
  plugin.registerCommand( "GCmdExport", CmdExport::creator );
  plugin.registerCommand( "GCmdExportAll", CmdExportAll::creator );
  plugin.registerCommand( "GCmdRestoreGui", CmdRestoreGui::creator );
  plugin.registerCommand( "GCmdSkinOn", CmdSkinOn::creator );
  plugin.registerCommand( "GCmdSkinOff", CmdSkinOff::creator );
  plugin.registerCommand( "GCmdTangentsOn", CmdTangentsOn::creator );
  plugin.registerCommand( "GCmdTangentsOff", CmdTangentsOff::creator );
  plugin.registerCommand( "GCmdBrowseFile", CmdBrowseFile::creator );
  plugin.registerCommand( "GCmdBrowseSkinFile", CmdBrowseSkinFile::creator );
  plugin.registerCommand( "GCmdProcessAnim", CmdProcessAnim::creator );
  plugin.registerCommand( "GCmdClearAnim", CmdClearAnim::creator );
  plugin.registerCommand( "GCmdAddAnim", CmdAddAnim::creator );
  plugin.registerCommand( "GCmdRemoveAnim", CmdRemoveAnim::creator );
  plugin.registerCommand( "GCmdSaveChar", CmdSaveChar::creator );

  //Define MEL commands
  MGlobal::executeCommand( mel_embedded );

  //Open GUI
  MGlobal::executeCommand( "GCmdCreateMenu" );
  //MGlobal::executeCommand( "GCmdOpen" );

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
  if (g_animation != NULL)
    delete g_animation;
  
  //Close GUI
  MGlobal::executeCommand( "GCmdDestroyMenu" );
  MGlobal::executeCommand( "GCmdClose" );

  //Deinit plugin
  MFnPlugin plugin( obj );
  plugin.deregisterCommand( "GCmdExport" );
  plugin.deregisterCommand( "GCmdExportAll" );
  plugin.deregisterCommand( "GCmdRestoreGui" );
  plugin.deregisterCommand( "GCmdSkinOn" );
  plugin.deregisterCommand( "GCmdSkinOff" );
  plugin.deregisterCommand( "GCmdTangentsOn" );
  plugin.deregisterCommand( "GCmdTangentsOff" );
  plugin.deregisterCommand( "GCmdBrowseFile" );
  plugin.deregisterCommand( "GCmdBrowseSkinFile" );
  plugin.deregisterCommand( "GCmdProcessAnim" );
  plugin.deregisterCommand( "GCmdClearAnim" );
  plugin.deregisterCommand( "GCmdAddAnim" );
  plugin.deregisterCommand( "GCmdRemoveAnim" );
  plugin.deregisterCommand( "GCmdSaveChar" );

	return status;
}