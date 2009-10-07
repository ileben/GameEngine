
#include "geMaya.h"
#include <maya/MFnPlugin.h> //Must only be included once!
#include <maya/MFileObject.h>
#include <maya/MBoundingBox.h>
#include <maya/MFnAmbientLight.h>

//This MEL script links the C++ code and the GUI.
#include "mel.embedded"


DEFINE_SERIAL_CLASS( MayaAnimDummy,  ClassID ( 0x3a64a0f5u, 0x8ba7, 0x43bd, 0x98bc949ecfa7fb74ull ));
DEFINE_SERIAL_CLASS( MayaClipDummy,  ClassID ( 0x251dd070u, 0x6d0b, 0x496a, 0x9a6eb10676a6da3eull ));
DEFINE_SERIAL_CLASS( MayaEventDummy, ClassID ( 0x00b10b6cu, 0x1332, 0x4c20, 0xae0b14e6440d55e5ull ));
DEFINE_SERIAL_CLASS( MayaSceneDummy, ClassID ( 0xd5c205c0u, 0xd846, 0x416b, 0x82ae7f337b15aee1ull ));


bool g_chkExportSkin = false;
bool g_chkExportTangents = false;
CharString g_outFileName;
CharString g_skinFileName;
CharString g_statusText;
Character *g_character = NULL;
Animation *g_animation = NULL;
ArrayList< MayaAnimDummy* > g_animations;
std::map< std::string, Actor3D* > g_mapNameToActor;

CharString g_sceneFileName;
MayaSceneDummy *g_scene = NULL;
Float g_worldScale = 1.0f;

/*
---------------------------------------
Helper functions
---------------------------------------*/

Float getWorldScale ()
{
  return g_worldScale;
}

File getProjectFolder ()
{
  //Find project folder
  MString workspace;
  MGlobal::executeCommand( MString( "workspace -q -rd;" ), workspace );
  return File( workspace.asChar() );
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

void selectListItem (const CharString &list, int index, bool scrollToIt = false)
{
  CharString cmd = "textScrollList -edit -selectIndexedItem " + CharString::FInt( index + 1 ) + " " + list;
  MGlobal::executeCommand( cmd.buffer() );
}

void scrollToListItem (const CharString &list, int index)
{
  CharString cmd = "textScrollList -edit -showIndexedItem " + CharString::FInt( index + 1 ) + " " + list;
  MGlobal::executeCommand( cmd.buffer() );
}

void clearList (const CharString &list)
{
  CharString cmd = "textScrollList -edit -removeAll " + list;
  MGlobal::executeCommand( cmd.buffer() );
}

Actor3D* findActorByName (const CharString &name)
{
  std::map< std::string, Actor3D* >::iterator it =
    g_mapNameToActor.find( name.buffer() );

  if (it != g_mapNameToActor.end())
    return it->second;

  return NULL;
}

bool findNodeByType (MFn::Type type, MDagPath &outPath, MDagPath *start)
{
  //Begin at start node if given
  MItDag dagIt( MItDag::kBreadthFirst, type );
  if (start != NULL)
    dagIt.reset( *start, MItDag::kBreadthFirst, type );

  //Walk the scene graph
  for (; !dagIt.isDone(); dagIt.next())
  {
    //Get path to the node
    MDagPath dagPath;
    dagIt.getPath( dagPath );

    //Skip intermediate objects
    MFnDagNode dagNode( dagPath );
    if (dagNode.isIntermediateObject()) continue;

    //Check type
    if (dagPath.hasFn( type ))
    {
      //Return match
      outPath.set( dagPath );
      return true;
    }
  }

  //Not found
  return false;
}

bool findNodeByName (const CharString &name, MDagPath &outPath)
{
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

    //Compare name
    CharString dagName = dagNode.name().asChar();
    if (dagName == name)
    {
      //Return match
      outPath.set( dagPath );
      return true;
    }
  }

  //Not found
  return false;
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

void findNodesInSelection (MFn::Type type,
                           ArrayList< MDagPath > &outPaths)
{
  //Get the active selection
  MSelectionList selList;
  MGlobal::getActiveSelectionList( selList );

  //Walk the selected nodes
  for (MItSelectionList selIt( selList ); !selIt.isDone(); selIt.next())
  {
    //Get the DAG path to the node
    MDagPath dagPath;
    selIt.getDagPath( dagPath );

    //Go down the tree at this selection
    MItDag dagSubIt; dagSubIt.reset( dagPath, MItDag::kBreadthFirst );
    for (; !dagSubIt.isDone(); dagSubIt.next())
    {
      MDagPath dagSubPath;
      dagSubIt.getPath( dagSubPath );

      //Skip intermediate objects and transform nodes
      MFnDagNode dagSubNode( dagSubPath );
      if (dagSubNode.isIntermediateObject()) continue;
      if (dagSubPath.apiType() == MFn::kTransform) continue;

      //Check for the requested api type
      if (dagSubPath.hasFn( type ))
        outPaths.pushBack( dagSubPath );

      break;
    }
  }
}

void findExportableNodesInSelection (ArrayList< MDagPath > &outPaths)
{
  findNodesInSelection( MFn::kMesh, outPaths );
  findNodesInSelection( MFn::kLight, outPaths );
  findNodesInSelection( MFn::kCamera, outPaths );
}

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

bool readPackageFile (File &file, Object **outObj, ClassPtr *outCls)
{
  //Open the file
  if (!file.open( FileAccess::Read, FileCondition::MustExist )) {
    setStatus( "Failed opening file!" );
    return false; }

  //Check the file signature
  SerializeManager sm;
  if (file.getSize() < sm.getSignatureSize()) {
    file.close();
    return false; }

  ByteString sig = file.read( sm.getSignatureSize() );
  if ((UintSize) sig.length() < sm.getSignatureSize()) {
    file.close();
    return false; }

  if ( ! sm.checkSignature( sig.buffer() )) {
    file.close();
    return false; }

  //Read the rest of the file
  ByteString data = file.read( file.getSize() - sm.getSignatureSize() );
  if (data.length() == 0) {
    file.close();
    return false; }

  //Close the file
  file.close();

  //Load the scene data
  *outObj = (Object*) sm.load( data.buffer(), outCls );
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
    if (! findNodeInSelection( MFn::kMesh, meshNode )) {
      setStatus( "Please select a mesh node!" );
      return MStatus::kFailure;
    }
    
    //Export character or static mesh
    Resource *res = NULL;
    if (g_chkExportSkin)
      res = exportCharacter( meshNode, g_chkExportTangents );
    else res = exportMesh( meshNode, g_chkExportTangents );

    //Make sure something was actually exported
    if (res == NULL) {
      setStatus( "Export failed!\\nAborted." );
      return MStatus::kFailure;
    }

    //Serialize and free data
    SerializeManager sm;
    sm.save( res, &outData, &outSize );
    delete res;

    //Write to file
    if (! writePackageFile( outData, outSize, File( outFileName ) ))
      return MStatus::kFailure;

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

  CharString removeRefFromName (const CharString &name)
  {
    int colon = name.findRev( ":" );
    if (colon == -1) return name;
    else return name.sub( colon+1 );
  }

  virtual MStatus doIt (const MArgList &args)
  {
    MStatus status;

    //Clear name-actor map
    g_mapNameToActor.clear();
    g_worldScale = 1.0;
    int meshID = 0;

    //Update status
    clearStatus();
    setStatus( "Exporting..." );
    trace( "\\nExport: starting..." );

    //Make sure a scene dummy exists
    if (g_scene == NULL) {
      setStatus( "No scene exists!" );
      return MStatus::kFailure;
    }

    //Get the file from the browse dialog
    MString result;
    MGlobal::executeCommand( "fileDialog -mode 1", result );
    if (result.length() == 0) {
      setStatus( "No file selected" );
      return MStatus::kFailure; }
    File outFile( result.asChar() );

    //Move to first frame
    MAnimControl animCtrl;
    animCtrl.setCurrentTime( MTime(0, MTime::uiUnit()) );

    //Create scene
    Scene3D *scene = new Scene3D;

    //Create root actor
    Actor3D *root = new Actor3D;
    scene->setRoot( root );

    //Walk the scene graph, searching for the scale ref dummy
    MItDag dagScaleIt( MItDag::kBreadthFirst );
    for ( ; !dagScaleIt.isDone(); dagScaleIt.next() )
    {
      //Get path to the node
      MDagPath dagPath;
      dagScaleIt.getPath( dagPath );

      //Skip intermediate objects
      MFnDagNode dagNode( dagPath );
      if (dagNode.isIntermediateObject())
        continue;

      //Check the name
      CharString name = dagNode.name().asChar();
      CharString nonRefName = removeRefFromName( name );
      if (nonRefName == "GameScale")
      {
        //Find the height of its bounding box
        MMatrix mworld = dagPath.inclusiveMatrix();
        MBoundingBox box = dagNode.boundingBox();
        box.transformUsing( mworld );
        Float h = (Float) box.height();

        //Adjust the exporting scale so that it matches 180 units
        g_worldScale = 180.0f / h;
        break;
      }
    }

    //Apply world scale to root object
    trace( "ExportScene: setting world scale to " + CharString::FFloat( g_worldScale ));
    root->scale( g_worldScale );

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

      //Find first transform above it. This uniquely identifies the object
      //since the instance duplicates will point to the same node
      MFnTransform transform( dagPath.transform() );
      CharString name = dagNode.partialPathName().asChar();
      
      //Find the export attribute
      MPlug plugExport = transform.findPlug( "GameExport", false, &status );
      if (status != MStatus::kSuccess) continue;
      
      //Skip if not marked for export
      bool exportable = false;
      plugExport.getValue( exportable );
      if (!exportable) continue;

      //Export actor by type
      Actor3D *outActor = NULL;
      if (dagPath.hasFn( MFn::kMesh ))
      {
        CharString resName;
        
        //Check if it's a mesh with skin
        MObject skin; bool hasSkin = false;
        if (findSkinForMesh( dagPath.node(), skin ))
          hasSkin = true;

        //Export material
        bool hasNormalMap = false;
        Material *material = exportMaterial( dagPath.node(), &hasNormalMap );
        if (material == NULL) {
          trace( "ExportScene: failed exporting material for '" + name + "'!" );
          continue;
        }

        //Find resource name plug
        //(custom attributes always go on the first transform node above)
        MPlug resNamePlug = transform.findPlug( "GameReference", false, &status );
        if (status == MStatus::kSuccess)
        {
          //Use the referenced resource name
          MString resNameStr;
          resNamePlug.getValue( resNameStr );
          resName = CharString( resNameStr.asChar() ) + ".pak";
        }
        else
        {
          //Use the node name for resource
          resName = name;
          
          //Export resource
          Resource *res = NULL;
          if (hasSkin) res = exportCharacter( dagPath.node(), hasNormalMap );
          else res = exportMesh( dagPath.node(), hasNormalMap );
          
          //Check if exported correctly
          if (res == NULL) {
            trace( "ExportScene: failed exporting mesh for '" + name + "'!" );
            continue;
          }
          
          //Name resource and add to scene
          res->setResourceName( resName );
          scene->resources.pushBack( res );
        }

        //Export actor
        Matrix4x4 matrix = exportMatrix( dagPath.inclusiveMatrix() );
        if (hasSkin)
        {
          //Create a new actor for this mesh
          SkinMeshActor *actor = new SkinMeshActor;
          actor->setCharacter( resName );
          actor->setMatrix( matrix );
          actor->setMaterial( material );
          actor->setParent( root );
          outActor = actor;

          //It is common to apply additional transformation to the root joint
          //via various constraints rather than the mesh object itself so grab that
          MObject rootJoint;
          if (findSkinJointRoot( skin, rootJoint ))
          {
            //Find the path to the root joint
            MDagPath jointPath;
            MFnIkJoint joint( rootJoint );
            joint.getPath( jointPath );

            //Add its world transform to the actor
            //Matrix4x4 mjoint = exportMatrix( jointPath.inclusiveMatrix() );
            Matrix4x4 mjoint = exportMatrix( jointPath.exclusiveMatrix() );
            actor->mulMatrixRight( mjoint );
          }
        }
        else
        {
          //Create a new actor for this mesh
          TriMeshActor *actor = new TriMeshActor;
          actor->setMesh( resName );
          actor->setMatrix( matrix );
          actor->setMaterial( material );
          actor->setParent( root );
          outActor = actor;
        }
      }
      else if (dagPath.hasFn( MFn::kAmbientLight ))
      {
        //Add color to scene ambient
        MFnAmbientLight ambLight( dagPath );
        Vector3 ambColor = exportColor( ambLight.color() ) * ambLight.intensity();
        scene->setAmbientColor( scene->getAmbientColor() + ambColor );
      }
      else if (dagPath.hasFn( MFn::kNonAmbientLight ))
      {
        //Export non-ambient light
        Light *light = exportLight( dagPath );
        if (light == NULL) continue;
        light->setParent( root );
        outActor = light;
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
        outActor = cam;
      }

      //Map Maya object name to exported actor
      if (outActor != NULL)
        g_mapNameToActor[ name.buffer() ] = outActor;

    }//walk the scene graph


    //Walk the list of animations
    for (UintSize a=0; a<g_scene->anims.size(); ++a)
    {
      //Export animation
      MayaAnimDummy *anim = g_scene->anims[ a ];
      Animation *outAnim = exportAnimation( 24, anim );

      //Check if exported correctly
      if (outAnim == NULL) {
        trace( "ExportScene: failed exporting animation '" + anim->name + "'!" );
        continue;
      }
      
      //Add to scne
      scene->animations.pushBack( outAnim );
    }

    //Export scene data
    void *outData = NULL;
    UintSize outSize = 0;
    SerializeManager sm;
    sm.save( scene, &outData, &outSize );

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
    Animation *newAnim = exportSkinAnimation( animStart, animEnd, 24 );
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
    Character *newChar = (Character*) sm.load( data.buffer(), &newCls );
    if (newCls != Class(Character) || newChar == NULL) {
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
    if (g_animation->getNumTracks()/2 !=
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
-------------------------------------------------
Scene animation controls
-------------------------------------------------*/

Float frameToTime (int key)
{
  MTime time;
  time.setUnit( MTime::uiUnit() );
  time.setValue( key );
  return (Float) time.as( MTime::kSeconds );
}

int timeToFrame (Float seconds)
{
  MTime time;
  time.setUnit( MTime::kSeconds );
  time.setValue( seconds );
  return (int) time.as( MTime::uiUnit() );
}

bool getSceneAnimInput (CharString *outName, int *outStartFrame, int *outEndFrame)
{
  CharString animName;
  int startFrame = 0;
  int endFrame = 0;


  if (outName != NULL)
  {
    //Make sure we have the animation name
    animName = getTextFieldText( "GTxtSceneAnimName" );
    if (animName.length() == 0) {
      setStatus( "Please enter animation name!" );
      return false;
    }

    //Return name
    *outName = animName;
  }

  if (outStartFrame != NULL)
  {
    //Make sure we have the start frame
    CharString startFrameStr = getTextFieldText( "GTxtSceneStartFrame" );
    if (startFrameStr.length() == 0) {
      setStatus( "Please enter start frame!" );
      return false;
    }

    //Parse start value
    int startLen = 0;
    startFrame = startFrameStr.parseIntegerAt( 0, &startLen );
    if (startLen == 0) {
      setStatus( "Invalid start value!" );
      return false;
    }
  }

  if (outStartFrame != NULL && outEndFrame != NULL)
  {
    //Make sure we have the end frame
    CharString endFrameStr = getTextFieldText( "GTxtSceneEndFrame" );
    if (endFrameStr.length() == 0) {
      setStatus( "Please enter end frame!" );
      return false;
    }

    //Parse end value
    int endLen;
    endFrame = endFrameStr.parseIntegerAt( 0, &endLen );
    if (endLen == 0) {
      setStatus( "Invalid end value!" );
      return false;
    }
  }

  if (outStartFrame != NULL && outEndFrame != NULL)
  {
    //Make sure end is after start
    if (endFrame < startFrame) {
      setStatus( "End frame before start frame!");
      return false;
    }
  }

  //Return start frame
  if (outStartFrame != NULL)
    *outStartFrame = startFrame;

  //Return end frame
  if (outEndFrame != NULL)
    *outEndFrame = endFrame;

  return true;
}

MayaAnimDummy* getSelectedSceneAnim()
{
  //Find selected index
  int sel = getListSelection( "GLstSceneAnims" );
  if (sel == -1) return NULL;

  //Make sure its in range
  if ((UintSize) sel >= g_scene->anims.size())
    return NULL;

  return g_scene->anims[ sel ];
}

MayaClipDummy* getSelectedSceneClip()
{
  //Find selected animation
  MayaAnimDummy *anim = getSelectedSceneAnim();
  if (anim == NULL) return NULL;

  //Find selected index
  int sel = getListSelection( "GLstSceneClips" );
  if (sel == -1) return NULL;

  //Make sure its in range
  if ((UintSize) sel >= anim->clips.size())
    return NULL;

  return anim->clips[ sel ];
}

MayaEventDummy* getSelectedSceneEvent()
{
  //Find selected animation
  MayaAnimDummy *anim = getSelectedSceneAnim();
  if (anim == NULL) return NULL;

  //Find selected index
  int sel = getListSelection( "GLstSceneEvents" );
  if (sel == -1) return NULL;

  //Make sure its in range
  if ((UintSize) sel >= anim->events.size())
    return NULL;

  return anim->events[ sel ];
}

template <class E>
void sortListItems (ObjPtrArrayList< E > *list)
{
  for (UintSize i=0; i<list->size(); ++i)
  {
    for (UintSize j=i+1; j < list->size(); ++j)
    {
      if ((*list->at( j )) < (*list->at( i )))
      {
        E *temp = list->at( i );
        list->at( i ) = list->at( j );
        list->at( j ) = temp;
      }
    }
  }
}
/*
void sortSceneAnims ()
{
  for (UintSize a1=0; a1<g_scene->anims.size(); ++a1)
  {
    for (UintSize a2=a1+1; a2 < g_scene->anims.size(); ++a2)
    {
      if (g_scene->anims[ a2 ]->startTime < g_scene->anims[ a1 ]->startTime)
      {
        MayaAnimDummy *atemp = g_scene->anims[ a1 ];
        g_scene->anims[ a1 ] = g_scene->anims[ a2 ];
        g_scene->anims[ a2 ] = atemp;
      }
    }
  }
}

void sortSceneClips (MayaAnimDummy *anim)
{
  for (UintSize c1=0; c1<anim->clips.size(); ++c1)
  {
    for (UintSize c2=c1+1; c2 < anim->clips.size(); ++c2)
    {
      if (anim->clips[ c2 ]->startTime < anim->clips[ c1 ]->startTime)
      {
        MayaClipDummy *ctemp = anim->clips[ c1 ];
        anim->clips[ c1 ] = anim->clips[ c2 ];
        anim->clips[ c2 ] = ctemp;
      }
    }
  }
}

void sortSceneEvents (MayaAnimDummy *anim)
{
  for (UintSize c1=0; c1<anim->clips.size(); ++c1)
  {
    for (UintSize c2=c1+1; c2 < anim->clips.size(); ++c2)
    {
      if (anim->clips[ c2 ]->startTime < anim->clips[ c1 ]->startTime)
      {
        MayaClipDummy *ctemp = anim->clips[ c1 ];
        anim->clips[ c1 ] = anim->clips[ c2 ];
        anim->clips[ c2 ] = ctemp;
      }
    }
  }
}
*/
CharString makeAnimTitleString (MayaAnimDummy *anim)
{
  int startFrame = timeToFrame( anim->startTime );
  int endFrame = timeToFrame( anim->endTime );
  return CharString::Format( "%d-%d %s", startFrame, endFrame, anim->name.buffer() );
}

CharString makeClipTitleString (MayaClipDummy *clip)
{
  int startFrame = timeToFrame( clip->startTime );
  int endFrame = timeToFrame( clip->endTime );
  return CharString::Format( "%d-%d %s", startFrame, endFrame, clip->nodePath.buffer() );
}

CharString makeEventTitleString (MayaEventDummy *evt)
{
  int frame = timeToFrame( evt->time );
  return CharString::Format( "%d %s", frame, evt->name.buffer() );
}

void updateSceneAnimsList (bool reselect = false)
{
  int sel = getListSelection( "GLstSceneAnims" );
  clearList( "GLstSceneAnims" );

  for (UintSize a=0; a<g_scene->anims.size(); ++a) {
    MayaAnimDummy *anim = g_scene->anims[ a ];
    appendListItem( "GLstSceneAnims", makeAnimTitleString( anim ) );
  }

  if (reselect && sel != -1)
    selectListItem( "GLstSceneAnims", sel, true );
}

void updateSceneClipsList (bool reselect = false)
{
  int sel = getListSelection( "GLstSceneClips" );
  clearList( "GLstSceneClips" );
  
  MayaAnimDummy *anim = getSelectedSceneAnim();
  if (anim == NULL) return;

  for (UintSize c=0; c<anim->clips.size(); ++c) {
    MayaClipDummy *clip = anim->clips[ c ];
    appendListItem( "GLstSceneClips", makeClipTitleString( clip ) );
  }

  if (reselect && sel != -1)
    selectListItem( "GLstSceneClips", sel, true );
}

void updateSceneEventsList (bool reselect = false)
{
  int sel = getListSelection( "GLstSceneEvents" );
  clearList( "GLstSceneEvents" );

  MayaAnimDummy *anim = getSelectedSceneAnim();
  if (anim == NULL) return;

  for (UintSize e=0; e<anim->events.size(); ++e) {
    MayaEventDummy *evt = anim->events[ e ];
    appendListItem( "GLstSceneEvents", makeEventTitleString( evt ) );
  }

  if (reselect && sel != -1)
    selectListItem( "GLstSceneEvents", sel, true );
}

void updateSceneAnimUI()
{
  setTextFieldText( "GTxtSceneFile", g_sceneFileName );
  updateSceneAnimsList();
  updateSceneClipsList();
  updateSceneEventsList();
}

/*
-----------------------------------------------
Command for browsing the character input file
-----------------------------------------------*/

class CmdSceneBrowse : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneBrowse(); }
  virtual MStatus doIt (const MArgList &args)
  {
    MString result;
    clearStatus();
    setStatus( "Loading scene..." );

    //Get the file from the browse dialog
    MGlobal::executeCommand( "fileDialog -mode 0", result );
    if (result.length() == 0) {
      setStatus( "No file selected" );
      return MStatus::kFailure; }

    File file( result.asChar() );

    //Load the scene data
    ClassPtr newCls = INVALID_CLASS_PTR;
    MayaSceneDummy *newScene = NULL;
    readPackageFile( File( result.asChar() ), (Object**) &newScene, &newCls );
    if (newCls != Class(MayaSceneDummy) || newScene == NULL) {
      setStatus( "Invalid file!" );
      return MStatus::kFailure; }

    //Swap old data with new
    if (g_scene != NULL) delete g_scene;
    g_sceneFileName = result.asChar();
    g_scene = newScene;

    //Update GUI
    updateSceneAnimUI();

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};


class CmdSceneNew : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneNew(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Creating new scene..." );

    //Create a new scene
    if (g_scene != NULL) delete g_scene;
    g_scene = new MayaSceneDummy;
    g_sceneFileName = "";

    //Update UI
    updateSceneAnimUI();

    setStatus( "Done.");
    return MStatus::kSuccess;
  }
};

class CmdSceneSave : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneSave(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Saving scene..." );

    //Make sure a scene exists and the filename was set
    if (g_scene == NULL || g_sceneFileName.length() == 0) {
      setStatus( "No scene loaded!" );
      return MStatus::kFailure;
    }

    //Save to the opened file
    return saveIt( File( g_sceneFileName ) );
  }

  MStatus saveIt (File &file)
  {
    //Export scene
    void *outData; UintSize outSize=0;
    SerializeManager sm;
    sm.save( g_scene, &outData, &outSize );
    
    //Make sure something was actually exported
    if (outSize == 0) {
      setStatus( "Zero data exported!" );
      return MStatus::kFailure;
    }

    //Open output file for writing
    if (!writePackageFile( outData, outSize, file )) {
      setStatus( "Failed writing to file." );
      return MStatus::kFailure;
    }

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdSceneSaveAs : public CmdSceneSave
{
public:
  static void* creator() { return new CmdSceneSaveAs(); }

  virtual MStatus doIt (const MArgList &args)
  {
    MString result;
    clearStatus();
    setStatus( "Saving scene..." );

    //Make sure a scene exists
    if (g_scene == NULL) {
      setStatus( "No scene exists!" );
      return MStatus::kFailure;
    }

    //Get the file from the browse dialog
    MGlobal::executeCommand( "fileDialog -mode 1", result );
    if (result.length() == 0) {
      setStatus( "No file selected" );
      return MStatus::kFailure; }

    //Save to the picked file
    MStatus status = saveIt( File( result.asChar() ) );
    if (status == MStatus::kSuccess)
    {
      //Set new scene filename
      g_sceneFileName = result.asChar();
      setTextFieldText( "GTxtSceneFile", g_sceneFileName );
    }

    return status;
  }
};


class CmdLstSceneAnimsSel : public MPxCommand
{ public:
  static void* creator() { return new CmdLstSceneAnimsSel(); }
  virtual MStatus doIt (const MArgList &args)
  {
    updateSceneClipsList();
    updateSceneEventsList();
    return MStatus::kSuccess;
  }
};

class CmdLstSceneClipsSel : public MPxCommand
{ public:
  static void* creator() { return new CmdLstSceneClipsSel(); }
  virtual MStatus doIt (const MArgList &args)
  {
    return MStatus::kSuccess;
  }
};

class CmdLstSceneAnimsDblClk : public MPxCommand
{ public:
  static void* creator() { return new CmdLstSceneAnimsDblClk(); }
  virtual MStatus doIt (const MArgList &args)
  {
    //Find selected animation
    MayaAnimDummy *anim = getSelectedSceneAnim();
    if (anim == NULL) return MStatus::kFailure;

    //Convert times to frames
    int startFrame = timeToFrame( anim->startTime );
    int endFrame = timeToFrame( anim->endTime );
    
    //Load into UI
    setTextFieldText( "GTxtSceneAnimName", anim->name );
    setTextFieldText( "GTxtSceneStartFrame", CharString::FInt( startFrame ) );
    setTextFieldText( "GTxtSceneEndFrame", CharString::FInt( endFrame ) );

    return MStatus::kSuccess;
  }
};

class CmdLstSceneClipsDblClk : public MPxCommand
{ public:
  static void* creator() { return new CmdLstSceneClipsDblClk(); }
  virtual MStatus doIt (const MArgList &args)
  {
    //Find selected clip
    MayaClipDummy *clip = getSelectedSceneClip();
    if (clip == NULL) return MStatus::kFailure;

    //Convert times to frames
    int startFrame = timeToFrame( clip->startTime );
    int endFrame = timeToFrame( clip->endTime );
    
    //Load into UI
    setTextFieldText( "GTxtSceneAnimName", "" );
    setTextFieldText( "GTxtSceneStartFrame", CharString::FInt( startFrame ) );
    setTextFieldText( "GTxtSceneEndFrame", CharString::FInt( endFrame ) );

    return MStatus::kSuccess;
  }
};

class CmdLstSceneEventsDblClk : public MPxCommand
{ public:
  static void* creator() { return new CmdLstSceneEventsDblClk(); }
  virtual MStatus doIt (const MArgList &args)
  {
    //Find selected event
    MayaEventDummy *evt = getSelectedSceneEvent();
    if (evt == NULL) return MStatus::kFailure;

    //Convert times to frames
    int frame = timeToFrame( evt->time );
    
    //Load into UI
    setTextFieldText( "GTxtSceneAnimName", evt->name );
    setTextFieldText( "GTxtSceneStartFrame", CharString::FInt( frame ) );
    setTextFieldText( "GTxtSceneEndFrame", "" );

    return MStatus::kSuccess;
  }
};


class CmdSceneSetAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneSetAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Setting animation properties..." );

    //Find selected animation
    MayaAnimDummy *anim = getSelectedSceneAnim();
    if (anim == NULL) {
      setStatus( "Please select an animation!" );
      return MStatus::kFailure;
    }

    //Get input values
    CharString animName;
    int startFrame;
    int endFrame;
    if (!getSceneAnimInput( &animName, &startFrame, &endFrame) )
      return MStatus::kFailure;

    //Update animation
    anim->name = animName;
    anim->startTime = frameToTime( startFrame );
    anim->endTime = frameToTime( endFrame );

    //Update GUI
    sortListItems( &g_scene->anims );
    int index = (int) g_scene->anims.indexOf( anim );

    updateSceneAnimsList();
    selectListItem( "GLstSceneAnims", index, true );

    updateSceneClipsList();
    updateSceneEventsList();

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdSceneSetClip : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneSetClip(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Setting clip properties..." );

    //Find selected clip
    MayaAnimDummy *anim = getSelectedSceneAnim();
    MayaClipDummy *clip = getSelectedSceneClip();
    if (clip == NULL) {
      setStatus( "Please select a clip!" );
      return MStatus::kFailure;
    }

    //Get the selected objects
    ArrayList< MDagPath > selection;
    findExportableNodesInSelection( selection );
    if (selection.empty()) {
      setStatus( "Please select an object!" );
      return MStatus::kFailure;
    }

    //Get the name of the first object
    MFnDagNode node( selection.first() );
    CharString nodePath = node.partialPathName().asChar();

    //Get input values
    int startFrame;
    int endFrame;
    if (!getSceneAnimInput( NULL, &startFrame, &endFrame) )
      return MStatus::kFailure;

    //Update clip
    clip->nodePath = nodePath;
    clip->startTime = frameToTime( startFrame );
    clip->endTime = frameToTime( endFrame );

    //Update GUI
    sortListItems( &anim->clips );
    int index = (int) anim->clips.indexOf( clip );

    updateSceneClipsList();
    selectListItem( "GLstSceneClips", index, true );

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};


class CmdSceneSetEvent : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneSetEvent(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Setting event properties..." );

    //Find selected clip
    MayaAnimDummy *anim = getSelectedSceneAnim();
    MayaEventDummy *evt = getSelectedSceneEvent();
    if (evt == NULL) {
      setStatus( "Please select an event!" );
      return MStatus::kFailure;
    }

    //Get input values
    CharString name; int frame;
    if (!getSceneAnimInput( &name, &frame, NULL ) )
      return MStatus::kFailure;

    //Update event
    evt->name = name;
    evt->time = frameToTime( frame );

    //Update GUI
    sortListItems( &anim->events );
    int index = (int) anim->events.indexOf( evt );

    updateSceneEventsList();
    selectListItem( "GLstSceneEvents", index, true );
    
    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};


class CmdSceneNewAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneNewAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Creating new animation..." );

    //Get input values
    CharString animName;
    int startFrame;
    int endFrame;
    if (!getSceneAnimInput( &animName, &startFrame, &endFrame) )
      return MStatus::kFailure;

    //Create new animation
    MayaAnimDummy *newAnim = new MayaAnimDummy;
    newAnim->name = animName;
    newAnim->startTime = frameToTime( startFrame );
    newAnim->endTime = frameToTime( endFrame );
    g_scene->anims.pushBack( newAnim );
    
    //Update GUI
    //sortSceneAnims();
    sortListItems( &g_scene->anims );
    int index = (int) g_scene->anims.indexOf( newAnim );

    updateSceneAnimsList();
    selectListItem( "GLstSceneAnims", index, true );

    updateSceneClipsList();
    updateSceneEventsList();

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdSceneNewClip : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneNewClip(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Creating new clip..." );

    //Get the selected animation
    MayaAnimDummy *anim = getSelectedSceneAnim();
    if (anim == NULL) {
      setStatus( "Please select an animation!" );
      return MStatus::kFailure;
    }

    //Get the selected objects
    ArrayList< MDagPath > selection;
    findExportableNodesInSelection( selection );
    if (selection.empty()) {
      setStatus( "Please select an object!" );
      return MStatus::kFailure;
    }

    //Get the name of the first object
    MFnDagNode node( selection.first() );
    CharString nodePath = node.partialPathName().asChar();

    //Get input values
    int startFrame;
    int endFrame;
    if (!getSceneAnimInput( NULL, &startFrame, &endFrame) )
      return MStatus::kFailure;

    //Create new clip
    MayaClipDummy *newClip = new MayaClipDummy;
    newClip->nodePath = nodePath;
    newClip->startTime = frameToTime( startFrame );
    newClip->endTime = frameToTime( endFrame );
    anim->clips.pushBack( newClip );
    
    //Update GUI
    //sortSceneClips( anim );
    sortListItems( &anim->clips );
    int index = (int) anim->clips.indexOf( newClip );

    updateSceneClipsList();
    selectListItem( "GLstSceneClips", (int) index, true );

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdSceneNewEvent : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneNewEvent(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();
    setStatus( "Creating new event..." );

    //Get the selected animation
    MayaAnimDummy *anim = getSelectedSceneAnim();
    if (anim == NULL) {
      setStatus( "Please select an animation!" );
      return MStatus::kFailure;
    }

    //Get input values
    CharString name; int frame;
    if (!getSceneAnimInput( &name, &frame, NULL) )
      return MStatus::kFailure;


    //Create new event
    MayaEventDummy *evt = new MayaEventDummy;
    evt->name = name;
    evt->time = frameToTime( frame );
    anim->events.pushBack( evt );

    //Update GUI
    //sortSceneEvents( anim );
    sortListItems( &anim->events );
    int index = (int) anim->events.indexOf( evt );

    updateSceneEventsList();
    selectListItem( "GLstSceneEvents", (int) index, true );

    setStatus( "Done." );
    return MStatus::kSuccess;
  }
};

class CmdSceneDelAnim : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneDelAnim(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();

    //Get the selected animation
    int sel = getListSelection( "GLstSceneAnims" );
    if (sel == -1) {
      setStatus( "Please select an animation!" );
      return MStatus::kFailure;
    }

    //Make sure its in valid range
    if ((UintSize) sel >= g_scene->anims.size()) {
      setStatus( "Invalid selection!" );
      return MStatus::kFailure;
    }

    //Remove animation from the scene
    delete g_scene->anims[ sel ];
    g_scene->anims.removeAt( sel );

    //Update GUI
    removeListItem( "GLstSceneAnims", sel );
    updateSceneClipsList();
    updateSceneEventsList();
    return MStatus::kSuccess;
  }
};

class CmdSceneDelClip : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneDelClip(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();

    //Get the selected animation
    MayaAnimDummy *anim = getSelectedSceneAnim();
    if (anim == NULL) {
      setStatus( "Please select an animation!" );
      return MStatus::kFailure;
    }

    //Get the selected clip
    int sel = getListSelection( "GLstSceneClips" );
    if (sel == -1) {
      setStatus( "Please select a clip!" );
      return MStatus::kFailure;
    }

    //Make sure its in valid range
    if ((UintSize) sel >= anim->clips.size()) {
      setStatus( "Invalid selection!" );
      return MStatus::kFailure;
    }

    //Remove clip from the animation
    delete anim->clips[ sel ];
    anim->clips.removeAt( sel );

    //Update GUI
    removeListItem( "GLstSceneClips", sel );
    return MStatus::kSuccess;
  }
};

class CmdSceneDelEvent : public MPxCommand
{ public:
  static void* creator() { return new CmdSceneDelEvent(); }
  virtual MStatus doIt (const MArgList &args)
  {
    clearStatus();

    //Get the selected animation
    MayaAnimDummy *anim = getSelectedSceneAnim();
    if (anim == NULL) {
      setStatus( "Please select an animation!" );
      return MStatus::kFailure;
    }

    //Get the selected clip
    int sel = getListSelection( "GLstSceneEvents" );
    if (sel == -1) {
      setStatus( "Please select an event!" );
      return MStatus::kFailure;
    }

    //Make sure its in valid range
    if ((UintSize) sel >= anim->events.size()) {
      setStatus( "Invalid selection!" );
      return MStatus::kFailure;
    }

    //Remove event from the animation
    delete anim->events[ sel ];
    anim->events.removeAt( sel );

    //Update GUI
    removeListItem( "GLstSceneEvents", sel );
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
    //Mesh

    if (g_chkExportSkin)
      MGlobal::executeCommand( "checkBox -edit -value true GChkSkin" );

    if (g_chkExportTangents)
      MGlobal::executeCommand( "checkBox -edit -value true GChkTangents" );

    if (g_outFileName.length() > 0)
      setTextFieldText( "GTxtFile", g_outFileName );

    //Animation

    if (g_animation != NULL)
    {
      disableAnimProcess();
      setTextFieldText( "GTxtAnimName", g_animation->name);
    }

    //Character

    if (g_skinFileName.length() > 0)
      setTextFieldText( "GTxtSkinFile", g_skinFileName );

    if (g_character != NULL)
    {
      for (UintSize a=0; a<g_character->anims.size(); ++a)
        appendListItem( "GLstAnim", g_character->anims[ a ]->name );
    }

    if (g_statusText.length() > 0)
      setTextLabel( "GTxtStatus", g_statusText );

    //Scene

    updateSceneAnimUI();

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
  
  g_sceneFileName = "";
  g_scene = new MayaSceneDummy;

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

  plugin.registerCommand( "GCmdSceneBrowse", CmdSceneBrowse::creator );
  plugin.registerCommand( "GCmdSceneNew", CmdSceneNew::creator );
  plugin.registerCommand( "GCmdSceneSave", CmdSceneSave::creator );
  plugin.registerCommand( "GCmdSceneSaveAs", CmdSceneSaveAs::creator );
  plugin.registerCommand( "GCmdLstSceneAnimsSel", CmdLstSceneAnimsSel::creator );
  plugin.registerCommand( "GCmdLstSceneAnimsDblClk", CmdLstSceneAnimsDblClk::creator );
  plugin.registerCommand( "GCmdLstSceneClipsSel", CmdLstSceneClipsSel::creator );
  plugin.registerCommand( "GCmdLstSceneClipsDblClk", CmdLstSceneClipsDblClk::creator );
  plugin.registerCommand( "GCmdLstSceneEventsDblClk", CmdLstSceneEventsDblClk::creator );
  plugin.registerCommand( "GCmdSceneNewAnim", CmdSceneNewAnim::creator );
  plugin.registerCommand( "GCmdSceneSetAnim", CmdSceneSetAnim::creator );
  plugin.registerCommand( "GCmdSceneDelAnim", CmdSceneDelAnim::creator );
  plugin.registerCommand( "GCmdSceneNewClip", CmdSceneNewClip::creator );
  plugin.registerCommand( "GCmdSceneSetClip", CmdSceneSetClip::creator );
  plugin.registerCommand( "GCmdSceneDelClip", CmdSceneDelClip::creator );
  plugin.registerCommand( "GCmdSceneNewEvent", CmdSceneNewEvent::creator );
  plugin.registerCommand( "GCmdSceneSetEvent", CmdSceneSetEvent::creator );
  plugin.registerCommand( "GCmdSceneDelEvent", CmdSceneDelEvent::creator );

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
  if (g_scene != NULL)
    delete g_scene;
  
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

  plugin.deregisterCommand( "GCmdSceneBrowse" );
  plugin.deregisterCommand( "GCmdSceneNew" );
  plugin.deregisterCommand( "GCmdSceneSave" );
  plugin.deregisterCommand( "GCmdSceneSaveAs" );
  plugin.deregisterCommand( "GCmdLstSceneAnimsSel" );
  plugin.deregisterCommand( "GCmdLstSceneAnimsDblClk" );
  plugin.deregisterCommand( "GCmdLstSceneClipsSel" );
  plugin.deregisterCommand( "GCmdLstSceneClipsDblClk" );
  plugin.deregisterCommand( "GCmdLstSceneEventsDblClk" );
  plugin.deregisterCommand( "GCmdSceneNewAnim" );
  plugin.deregisterCommand( "GCmdSceneSetAnim" );
  plugin.deregisterCommand( "GCmdSceneDelAnim" );
  plugin.deregisterCommand( "GCmdSceneNewClip" );
  plugin.deregisterCommand( "GCmdSceneSetClip" );
  plugin.deregisterCommand( "GCmdSceneDelClip" );
  plugin.deregisterCommand( "GCmdSceneNewEvent" );
  plugin.deregisterCommand( "GCmdSceneSetEvent" );
  plugin.deregisterCommand( "GCmdSceneDelEvent" );

	return status;
}